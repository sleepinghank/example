use std::{sync::atomic::Ordering, thread::sleep};
use std::sync::mpsc::SyncSender;
use std::thread;
use std::time::Duration;
use anyhow::anyhow;
use rdev::listen as listen_event;
use serialport::available_ports;
use crate::global::{SYS_KEY_QUIT, SYS_KEY_STATUS};
use anyhow::Result;

use crate::force_model::{
    FrameParser,
    RawData,
    TrainingData,
};

/// 震动缓冲配置
#[derive(Debug, Clone)]
struct VibrationBufferConfig {
    threshold: usize,  // 需要连续多少帧相同状态才触发
    enabled: bool,     // 是否启用缓冲
}

impl Default for VibrationBufferConfig {
    fn default() -> Self {
        Self {
            threshold: 2,
            enabled: true,
        }
    }
}

/// 震动缓冲状态
struct VibrationBuffer {
    config: VibrationBufferConfig,
    prediction_buffer: usize,  // 预测状态缓冲
    buffer_count: usize,       // 缓冲计数器
    current_status: usize,     // 当前状态
}

impl VibrationBuffer {
    fn new(config: VibrationBufferConfig) -> Self {
        Self {
            config,
            prediction_buffer: 0,
            buffer_count: 0,
            current_status: 0,
        }
    }
    
    /// 更新预测状态并返回是否应该触发震动
    fn update(&mut self, prediction: usize) -> bool {
        if !self.config.enabled {
            // 如果缓冲被禁用，直接比较
            if prediction != self.current_status {
                self.current_status = prediction;
                return true;
            }
            return false;
        }
        
        // 缓冲机制：需要连续多帧相同状态才触发
        if prediction == self.prediction_buffer {
            self.buffer_count += 1;
        } else {
            // 状态改变，重置缓冲
            self.prediction_buffer = prediction;
            self.buffer_count = 1;
        }
        
        // 检查是否达到缓冲阈值且状态与当前状态不同
        if self.buffer_count >= self.config.threshold && prediction != self.current_status {
            self.current_status = prediction;
            return true;
        }
        
        false
    }
    
    /// 获取当前缓冲状态信息
    fn get_status_info(&self) -> (usize, usize, usize, usize) {
        (self.prediction_buffer, self.buffer_count, self.current_status, self.config.threshold)
    }
    
    /// 重置缓冲状态
    fn reset(&mut self) {
        self.prediction_buffer = 0;
        self.buffer_count = 0;
        self.current_status = 0;
    }
    
    /// 更新配置
    fn update_config(&mut self, config: VibrationBufferConfig) {
        self.config = config;
        self.reset(); // 重置状态以应用新配置
    }
}
pub fn pre_data() -> Result<()>{
    let mut force_l_avg = 0.0;
    let mut force_r_avg = 0.0;
    let mut base_count = 0;
    
    // 创建震动缓冲器，配置为需要连续2帧相同状态才触发
    let mut vibration_buffer = VibrationBuffer::new(VibrationBufferConfig {
        threshold: 2,
        enabled: true,
    });
    let priors = vec![0.4939106901217862, 0.5060893098782138];
    let sigmas = vec![
        vec![209672.5458046464, 46273.3704997112, 4.629272464034061, 1916.1953599100862, 1520.0326276901556],
        vec![241136.3079324748, 46298.57549088368, 7.322438179891191, 81691.8801249882, 100694.46098231901]
    ];
    let thetas = vec![
        vec![939.6821917808219, 355.7287671232877, 8.6, 36.26575342465753, 8.378082191780821],
        vec![787.7219251336899, 356.20588235294116, 9.109625668449198, 216.63101604278074, 254.2620320855615]
    ];

    // Estimator:
    let clf = crate::gaussian_nb::GaussianNB::new(priors, sigmas, thetas);
    thread::spawn(|| {
        if let Err(e) = listen_keyboard() {
            eprintln!("Error listening to keyboard: {}", e);
        }
    });
    let (tx, rx) = std::sync::mpsc::channel();
    let (tx2, rx2) = std::sync::mpsc::channel();
    let port_name = "COM8";
    if let Err(e) = crate::force_model::uart_data::read_from_serial_port(&port_name, tx,rx2) {
        eprintln!("Error reading from serial port: {}", e);
        return Ok(())
    }
    println!("Listening on port: {}", port_name);
    let mut parser = FrameParser::new();
    let mut data_array: Vec<TrainingData> = Vec::new();
    loop {
        match rx.try_recv() {
            Ok(data) => {
                if let Some(data) = parser.parse_byte(data) {
                    let data = RawData::from(data.to_vec());
                    let training_data = TrainingData::new(data, SYS_KEY_STATUS.load(Ordering::Relaxed));
                    // data_array.push(training_data.clone());
                    // println!("Received data: {:?}", training_data);
                    if training_data.raw_data.x == 0 && training_data.raw_data.y == 0{
                        force_l_avg += training_data.raw_data.force_l as f64;
                        force_r_avg += training_data.raw_data.force_r as f64;
                        base_count +=1;
                        continue;
                    } else {
                        if base_count > 0 {
                            force_l_avg /= base_count as f64;
                            force_r_avg /= base_count as f64;
                        }
                        base_count = 0;
                    }
                    let features:Vec<f64> = vec![training_data.raw_data.x as f64,
                                                 training_data.raw_data.y as f64, training_data.raw_data.size as f64, training_data.raw_data.force_l as f64 - force_l_avg, training_data.raw_data.force_r as f64 - force_r_avg];
                    // Get class prediction:
                    let prediction = clf.predict(features.as_slice());
                    println!("Predicted class: #{}", prediction);
                    // // 使用震动缓冲器更新状态
                    // let should_vibrate = vibration_buffer.update(prediction);
                    
                    // // 调试信息：显示缓冲状态
                    // let (buffer_pred, buffer_count, current_status, threshold) = vibration_buffer.get_status_info();
                    // if vibration_buffer.config.debug_mode {
                    //     println!("[DEBUG] Predicted: {}, Buffer: {}, Count: {}/{}, Status: {}", 
                    //              prediction, buffer_pred, buffer_count, threshold, current_status);
                    // }
                    
                    // // 检查是否应该触发震动
                    // if should_vibrate {
                    //     // 发送震动指令
                    //     tx2.send(0x01).expect("TODO: panic message");
                    //     println!("*** VIBRATION TRIGGERED *** button down:{:?} (buffered for {} frames)", prediction, buffer_count);
                    // }
                }
            },
            Err(_) => {
                // No data received yet, continue waiting
                thread::sleep(std::time::Duration::from_millis(100));
            }
        }
        // 使用震动缓冲器更新状态
        let should_vibrate = vibration_buffer.update(SYS_KEY_QUIT.load(Ordering::Relaxed) as usize);

        // 检查是否应该触发震动
        if should_vibrate {
            // 发送震动指令
            tx2.send(0x01).expect("TODO: panic message");
            println!("*** VIBRATION TRIGGERED ***");
        }
        // if SYS_KEY_QUIT.load(Ordering::Relaxed) {
        //     println!("Keyboard quit signal received, exiting...");
        //     break;
        // }
        sleep(Duration::from_millis(2));
    }
    // 保存数据到文件

    let json_storage = crate::json_storage::JsonStorage::new_temp("./resources/","data","json");
    json_storage.save(&data_array)?;
    println!("Data saved successfully to file.");
    Ok(())
}
pub fn listen_keyboard() -> anyhow::Result<()> {
    if let Err(err) = listen_event(move |event: rdev::Event| {
        let event_type = event.event_type;
        let name = event.name;
        let time = event.time;
        match event_type {
            rdev::EventType::KeyPress(key) => {
                // sender.send(key).unwrap();
                // println!("KeyPress: {:?},name:{:?},time:{:?}", key,name,time);
                SYS_KEY_STATUS.store(true,Ordering::Relaxed);
                if key == rdev::Key::Escape {
                    SYS_KEY_QUIT.store(true, Ordering::Relaxed);
                }
            }
            rdev::EventType::KeyRelease(key) => {
                // sender.send(key).unwrap();
                // println!("KeyPress: {:?},name:{:?},time:{:?}", key,name,time);
                SYS_KEY_STATUS.store(false,Ordering::Relaxed);
                if key == rdev::Key::Escape {
                    SYS_KEY_QUIT.store(false, Ordering::Relaxed);
                }
            }
            _ => {}
        }
    }) {
        return Err(anyhow!("Error: {:?}", err));
    }
    Ok(())
}
