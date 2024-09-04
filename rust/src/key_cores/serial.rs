use std::{sync::mpsc, thread};

use anyhow::Result;

/// 获取可用的串口
pub fn get_available_serialport() -> Result<Vec<String>>{
    let ports = serialport::available_ports()?;
    Ok(ports.into_iter().map(|port| port.port_name).collect())
}

/// 读取给定串口号数据，以换行为间隔，读取每行数据，返回mpsc管道
pub fn read_from_serial_port(port_name: &str,baud_rate: u32, tx: mpsc::Sender<String>) -> Result<()> {
    let mut port = serialport::new(port_name, baud_rate)
        .open()?;
    
    thread::spawn(move || {
        println!("start read");
        // let mut message = "".to_string();
        // 创建大批量缓冲区
        let mut serial_buf: Vec<u8> = vec![0; 10];
        loop {
            match port.read(serial_buf.as_mut_slice()) {
                Ok(size) => {
                    if size > 0 {
                        // 转为string
                        let data = String::from_utf8_lossy(&serial_buf[..size]).to_string();
                        let mut buf = BUF_STR.write().unwrap(); // 获取写锁
                        buf.push_str(&data);
                        // // 拼接字符串
                        // message.push_str(&data);
                        // // 判断是否有换行符
                        // if message.contains("\r\n") {
                        //     // 以换行符分割字符串
                        //     let messages: Vec<&str> = message.split("\r\n").collect();
                        //     // 遍历除开最后一个字符串之前的字符
                        //     // let last = messages.last().cloned().unwrap_or_default().to_string();
                        //     // 发送数据
                        //     tx.send(messages.get(0).unwrap().to_string()).unwrap();
                        //     // 清空最后一个 换行符 之前的数据
                        //     message.clear();
                        // }
                    }
                },
                Err(e) => {
                    eprintln!("Error reading from serial port: {}", e);
                    break;
                }
            }
            // 休眠100ms
            thread::sleep(std::time::Duration::from_millis(100));
        }
    });
    Ok(())
}

use std::time::Duration;

use thread::sleep;
use crate::key_cores::BUF_STR;

pub fn test_read_from_serial_port() {
    let (tx, rx) = mpsc::channel();
    read_from_serial_port("COM7", 921600, tx).unwrap();
    loop {
        // 无限循环，等待数据
        match rx.recv() {
            Ok(data) => {
                // 转为string
                println!("{:?}", data);
            },
            Err(e) => {
                eprintln!("Error reading from serial port: {}", e);
                break;
            }
        }
        // 休眠500ms
        sleep(Duration::from_millis(100));
    }
}

#[cfg(test)]
mod tests {
    use std::time::Duration;

    use thread::sleep;

    use super::*;

    #[test]
    fn test_get_available_serialport() {
        get_available_serialport().unwrap();
    }

    #[test]
    fn test_read_from_serial_port() {
        let (tx, rx) = mpsc::channel();
        read_from_serial_port("COM6", 115200, tx).unwrap();
        loop {
            // 无限循环，等待数据
            match rx.recv() {
                Ok(data) => {
                    println!("{:?}", data);
                },
                Err(e) => {
                    eprintln!("Error reading from serial port: {}", e);
                    break;
                }
            }
            // 休眠500ms
            sleep(Duration::from_millis(100));
        }
    }
}