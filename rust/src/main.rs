

use std::env;
use std::sync::mpsc::sync_channel;
use tokio::process::Command;
use tokio::sync::mpsc;
use crate::os::WINDOW_SENDER;
use get_selected_text::get_selected_text;
use windows::Devices::Bluetooth::{BluetoothConnectionStatus, BluetoothLEDevice};
use windows::Devices::Enumeration::{DeviceInformation, DeviceInformationUpdate, DeviceWatcher};
use windows::Foundation::TypedEventHandler;
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
mod letcode;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // get_device_info().await;
    // info!("start pair 00000000000");
    // let handler: TypedEventHandler<DeviceWatcher, DeviceInformation> =
    //     TypedEventHandler::new(move |_sender, args: &Option<DeviceInformation>| {
    //         println!("DeviceWatcher 11111: {:?}", args);
    //         if let Some(args) = args {
    //             // on_connected(args);
    //         }
    //         Ok(())
    //     });
    // let handler_update: TypedEventHandler<DeviceWatcher, DeviceInformationUpdate> =
    //     TypedEventHandler::new(move |_sender, args: &Option<DeviceInformationUpdate>| {
    //         println!("DeviceWatcher 2222: {:?}", args);
    //         if let Some(args) = args {
    //             // on_removed(args);
    //         }
    //         Ok(())
    //     });
    //
    // let pair= BluetoothLEDevice::GetDeviceSelectorFromConnectionStatus(
    //     BluetoothConnectionStatus::Connected)?;
    // let watcher = DeviceInformation::CreateWatcherAqsFilter(&pair).unwrap();
    // watcher.Added(&handler).unwrap();
    // watcher.Updated(&handler_update).unwrap();
    // watcher.Removed(&handler_update).unwrap();
    // watcher.Start()?;

    // {
    //     let mut write_lock = self.token.write().unwrap();
    //     *write_lock = Some(token);
    //     let mut update_lock = self.update_token.write().unwrap();
    //     *update_lock = Some(token_update);
    //     let mut removed_lock = self.removed_token.write().unwrap();
    //     *removed_lock = Some(removed);
    // }
    // loop {
    //     tokio::time::sleep(std::time::Duration::from_secs(1)).await;
    // }
    Ok(())
}

