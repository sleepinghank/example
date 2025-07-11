

use std::{env, thread};
use std::sync::mpsc::sync_channel;
use tokio::process::Command;
use tokio::sync::mpsc;
use crate::os::WINDOW_SENDER;
use get_selected_text::get_selected_text;
use serialport::available_ports;
use windows::Devices::Bluetooth::{BluetoothConnectionStatus, BluetoothLEDevice};
use windows::Devices::Enumeration::{DeviceInformation, DeviceInformationUpdate, DeviceWatcher};
use windows::Foundation::TypedEventHandler;
use windows::Win32::System::SystemInformation::{
    GetLogicalProcessorInformationEx, GetSystemInfo, RelationAll, RelationProcessorCore,
    SYSTEM_INFO, SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX,
};
use crate::force_model::TrainingData;
use crate::key_cores::cli::listen_keyboard;

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
mod force_model;
mod global;
mod json_storage;
mod gaussian_nb;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    force_model::preprocess_data::pre_data()?;
    Ok(())
}

