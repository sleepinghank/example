mod port;
mod touch;
mod kalman;
mod encrypt;

use std::time::{SystemTime, UNIX_EPOCH};
use std::process::Command;

fn main() {
    println!("begin-----------------------------");
    // port::receive_data::test_receive_data();
    // touch::touch().unwrap();
    // let tim = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_secs() as u32;
    // println!("tim: {}", tim);
    // match SystemTime::now().duration_since(UNIX_EPOCH) {
    //     Ok(n) => println!("1970-01-01 00:00:00 UTC was {} seconds ago!", n.as_secs()),
    //     Err(_) => panic!("SystemTime before UNIX EPOCH!"),
    // }

    //     uint8_t a = 51;
    //     uint8_t b = 172;
    //     uint8_t c = a^b;
    //     printf("c:%d\n",c);

    let output = Command::new("./FirmwareUpdateTool.exe")
    .spawn()
    .expect("Failed to execute command");

    // if output.status.success() {
    //     println!("Command executed successfully!");
    //     let stdout = String::from_utf8_lossy(&output.stdout);
    //     println!("Output: {}", stdout);
    // } else {
    //     let stderr = String::from_utf8_lossy(&output.stderr);
    //     eprintln!("Error executing command: {}", stderr);
    // }
    println!("end-----------------------------");
}
