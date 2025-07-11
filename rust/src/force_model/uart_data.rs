use std::io::Write;
use std::sync::mpsc;
use std::thread;
use std::thread::sleep;
use serialport;

pub fn read_from_serial_port(port_name: &str, tx: mpsc::Sender<u8>,rx: mpsc::Receiver<u8>) -> anyhow::Result<()> {

    let mut port = serialport::new(port_name, 921600)
        // .timeout(Duration::from_millis(10))
        .open()?;

    thread::spawn(move || {
        let mut buf = [0u8; 6]; // 缓冲区大小可根据需要调整

        loop {
            match port.read(&mut buf) {
                Ok(size) => {
                    if size > 0 {
                        let data = buf[..size].to_vec();
                        for byte in data {
                            if tx.send(byte).is_err() {
                                // 如果发送失败，可能是接收端已经关闭
                                return;
                            }
                        }
                    }
                },
                Err(e) => {
                    // eprintln!("Error reading from serial port: {}", e);
                    // break;
                }
            }
            match rx.try_recv() {
                Ok(data) => {
                    let a = [data];
                    port.write(&a).expect("TODO: panic message");
                },
                Err(_) => {
                    // No data received yet, continue waiting
                }
            }
            thread::sleep(std::time::Duration::from_millis(9));
        }
    });
    // loop {
    //     sleep(std::time::Duration::from_secs(5));
    //     let a = [0x01];
    //     port.write(&a).expect("TODO: panic message");
    //     println!("Sent data: {:?}", a);
    // }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use serialport::available_ports;

    #[test]
    fn test_read_from_serial_port() {
        // let (tx, rx) = mpsc::channel();
        // // let port_name = available_ports().unwrap().first().unwrap().port_name.clone();
        // let port_name = "COM8";
        //
        // // assert!(read_from_serial_port(&port_name, tx).is_ok());
        //
        // // Allow some time for the thread to start
        // // thread::sleep(std::time::Duration::from_secs(1));
        //
        // // Check if we can receive data
        // // assert!(rx.try_recv().is_err());
        //
        // loop {
        //     match rx.try_recv() {
        //         Ok(data) => {
        //             println!("Received data: {:?}", data);
        //             break; // Exit loop after receiving data
        //         },
        //         Err(_) => {
        //             // No data received yet, continue waiting
        //             thread::sleep(std::time::Duration::from_millis(100));
        //         }
        //     }
        // }
    }

}