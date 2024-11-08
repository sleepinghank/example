

use std::env;
use std::sync::mpsc::sync_channel;
use tokio::process::Command;
use tokio::sync::mpsc;
use crate::os::WINDOW_SENDER;
use get_selected_text::get_selected_text;
use windows::Win32::System::SystemInformation::{
    GetLogicalProcessorInformationEx, GetSystemInfo, RelationAll, RelationProcessorCore,
    SYSTEM_INFO, SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX,
};

mod port;
mod touch;
mod kalman;
mod encrypt;
mod os;
mod openai;
mod file_test;
mod serial; 
mod key_cores;
mod registry;
mod sysinfo;
mod downloader;
mod select_text;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // key_cores::start().unwrap();
    // key_cores::serial::test_read_from_serial_port();
    key_cores::cli::test_listen_keyboard();
    // key_cores::start().unwrap();
    // port::receive_data::test_receive_data();

//     openai::chat::test(r"请将下面这段翻译成中文：Typing proficiency is an essential skill in the modern world, enabling efficient communication and productivity.
// Practice regularly to increase your speed and accuracy. Start with short passages like this one,
// focusing on keeping your fingers on the home row: ASDF for the left hand and JKL; for the right.
// As you type, try not to look at the keyboard; instead, let your muscle memory guide your fingers.".to_string()).await?;

    // while true {
    //     let a = openai::chat::local_ai_health().await;
    //     println!("a: {:?}", a);
    //     tokio::time::sleep(std::time::Duration::from_secs(1)).await;
    // }
//     println!("result: {:?}", result);
//     let a = sysinfo::get_nvidia_idx()?;
//     println!("a: {:?}", a);


    // let cpu_info = sysinfo::CpuInfo::get_cpu_info().unwrap();
    // println!("cpu_info: {:?}", cpu_info);

    // let gpu_info = sysinfo::GpuInfo::get_gpu_info().unwrap();
    // println!("gpu_info: {:?}", gpu_info);

    // let install_cuda = sysinfo::detect_cuda().unwrap();
    // println!("install_cuda: {:?}", install_cuda);


    // tokio::time::sleep(std::time::Duration::from_secs(100)).await;

    // sysinfo::open_llama().await?;

    // sysinfo::commd_kill_test().await?;
    // sysinfo::check_env().await?;
    // loop {
    //     // select_text::get_selected();
    //     // match select_text::get_selected() {
    //     //     Ok(text) => {
    //     //         println!("selected text: {}", text);
    //     //     },
    //     //     Err(e) => {
    //     //         println!("error occurred while getting the selected text,{:?}", e);
    //     //     }
            
    //     // }
      
    //     tokio::time::sleep(std::time::Duration::from_secs(1)).await;
    // }

    // let (sync_sender, key_rx) = sync_channel(1024);
    // std::thread::spawn(move || {
    //     select_text::listen_mouse(sync_sender).unwrap();
    // });
    //
    // loop {
    //     match key_rx.recv() {
    //         Ok((x,y)) => {
    //             match get_selected_text() {
    //                 Ok(selected_text) => {
    //                     println!("pos:({:?},{:?})selected text: {}", x,y,selected_text);
    //                 },
    //                 Err(e) => {
    //                     println!("error occurred while getting the selected text,{:?}", e);
    //                 }
    //             }
    //         }
    //         Err(e) => {
    //             eprintln!("Error reading from serial port: {}", e);
    //             break;
    //         }
    //     }
    // }
    // match sysinfo::CpuInfo::get_cpu_info() {
    //     Ok(cpu_info) => {
    //         println!("cpu_info: {:?}", cpu_info);
    //     },
    //     Err(e) => {
    //         println!("error occurred while getting the cpu info,{:?}", e);
    //     }
    // }
    // use std::error::Error;
    // use winreg::RegKey;
    //
    //
    // let hkey = RegKey::predef(winreg::enums::HKEY_LOCAL_MACHINE);
    // let hardware_key = hkey.open_subkey_with_flags(
    //     r"Hardware\Description\System\CentralProcessor\0",
    //     winreg::enums::KEY_READ,
    // )?;
    //
    //
    // if let Ok(version) = hardware_key.get_value::<String, _>("ProcessorNameString") {
    //     println!("cpu_name {}", version);
    // }

    // std::thread::sleep(std::time::Duration::from_secs(100));
    Ok(())
}


