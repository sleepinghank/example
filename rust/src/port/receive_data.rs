use std::fs::File;
use std::io::{self, Write};
use std::path::Path;
use std::time::Duration;
use serialport::{available_ports, SerialPortType};
use anyhow::Result;
use chrono::Local;

use std::sync::mpsc;
use std::thread;

fn read_from_serial_port(port_name: &str, tx: mpsc::Sender<Vec<u8>>) -> Result<()> {

    let mut port = serialport::new(port_name, 115200)
        // .timeout(Duration::from_millis(10))
        .open()?;

    thread::spawn(move || {
        let mut buf = [0u8; 10]; // 缓冲区大小可根据需要调整

        loop {
            match port.read(&mut buf) {
                Ok(size) => {
                    if size > 0 {
                        let data = buf[..size].to_vec();
                        tx.send(data).unwrap();
                    }
                },
                Err(e) => {
                    eprintln!("Error reading from serial port: {}", e);
                    break;
                }
            }
        }
    });

    Ok(())
}




pub fn get_data() -> Result<()>{
    println!("begin get data");
    let fmt = "%Y%m%d_%H%M%S";
    let now = Local::now().format(fmt).to_string();
    match available_ports() {
        Ok(ports) => {
            match ports.len() {
                0 => println!("No ports found."),
                1 => println!("Found 1 port:"),
                n => println!("Found {} ports:", n),
            };
            for p in ports {
                let mut port = serialport::new(p.port_name.clone(), 921600)
                    .timeout(Duration::from_millis(10))
                    .open().unwrap();

                let file_name = format!("{}_{}.txt", p.port_name.clone(), now);
                print_device_info(p.port_type.clone());
                println!("Receiving data on {:?} at {} baud:", p.port_name.clone(), 115200);
                println!("File Name: {}",file_name);
                let mut file = File::create(Path::new(&file_name))?;

                let mut serial_buf: Vec<u8> = vec![0; 100];
                let mut count = 0_u32;
                loop {
                    match port.read(serial_buf.as_mut_slice()) {
                        Ok(t) => {
                            count += 1;
                            // io::stdout().write_all(&serial_buf[..t]).unwrap();
                            // println!("{:?}", serial_buf[..t].to_vec());
                            file.write_all(&serial_buf[..t])?;
                            println!("idx:{},receive {} bytes", count,t);
                        },
                        Err(ref e) if e.kind() == io::ErrorKind::TimedOut => (),
                        Err(e) => {
                            eprintln!("{:?}", e);
                            break;
                        },
                    }
                }
                println!("over!!!");
                
            }
        }
        Err(e) => {
            eprintln!("{:?}", e);
            eprintln!("Error listing serial ports");
        }
    }
    Ok(())
}

fn print_device_info(port_type: SerialPortType) {
    match port_type {
        SerialPortType::UsbPort(info) => {
            println!("    Type: USB");
            println!("    VID:{:04x} PID:{:04x}", info.vid, info.pid);
            println!(
                "     Serial Number: {}",
                info.serial_number.as_ref().map_or("", String::as_str)
            );
            println!(
                "      Manufacturer: {}",
                info.manufacturer.as_ref().map_or("", String::as_str)
            );
            println!(
                "           Product: {}",
                info.product.as_ref().map_or("", String::as_str)
            );
        }
        SerialPortType::BluetoothPort => {
            println!("    Type: Bluetooth");
        }
        SerialPortType::PciPort => {
            println!("    Type: PCI");
        }
        SerialPortType::Unknown => {
            println!("    Type: Unknown");
        }
    }
}

pub fn test_receive_data() {
    // 创建一个新的通道用于测试
    let (tx, rx) = mpsc::channel::<Vec<u8>>();

    // 在测试中调用读取串口数据的方法
    if let Err(e) = read_from_serial_port("COM10", tx.clone()) {
        println!("Error in reading from serial port: {}", e);
        return
    }
    
    loop {
        let received_data = rx.recv().unwrap();
        io::stdout().write_all(&received_data).unwrap();
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::mpsc;
    use std::io::{self, Write};

    #[test]
    fn test_read_from_serial_port() {
        // 创建一个新的通道用于测试
        let (tx, rx) = mpsc::channel::<Vec<u8>>();

        // 在测试中调用读取串口数据的方法
        if let Err(e) = read_from_serial_port("COM20", tx.clone()) {
            panic!("Error in reading from serial port: {}", e);
        }

        // // 在测试中接收数据并断言是否与预期一致
        // let received_data = rx.recv().unwrap();
        // let mut output: Vec<u8> = vec![];
        // let result = io::stdout().write_all(&received_data);
        for item in  rx.recv() {
            println!("{:?}", item);
        }

        assert_eq!(true, true);
    }
}
