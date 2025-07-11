


use crossterm::event::{self, Event, KeyCode, KeyEvent};
use anyhow::{anyhow, Result};

use std::{sync::mpsc::SyncSender, thread};

use rdev::listen as listen_event;

pub fn listen_keyboard(sender: SyncSender<rdev::Key>) -> Result<()>{
    if let Err(err) = listen_event(move |event: rdev::Event| {
        callback(event, sender.clone());
    }) {
        return Err(anyhow!("Error: {:?}", err));
    }
    Ok(())
}

fn callback(event: rdev::Event, sender: SyncSender<rdev::Key>) {
    let event_type = event.event_type;
    let name = event.name;
    let time = event.time;
    match event_type {
        rdev::EventType::KeyPress(key) => {
            // sender.send(key).unwrap();
            // println!("KeyPress: {:?},name:{:?},time:{:?}", key,name,time);
        }
        rdev::EventType::KeyRelease(key) => {
            // sender.send(key).unwrap();
            // println!("KeyPress: {:?},name:{:?},time:{:?}", key,name,time);
        }
        rdev::EventType::ButtonPress(b) => {
            // println!("ButtonPress: {:?},name:{:?},time:{:?}", b,name,time);
        },
        rdev::EventType::ButtonRelease(b) => {
            // println!("ButtonPress: {:?},name:{:?},time:{:?}", b,name,time);
        },
        rdev::EventType::MouseMove {x,y  } => {
            // println!("MouseMove: {:?}{:?}", x,y);
        },
        rdev::EventType::Wheel {delta_x,delta_y} => {
            // println!("Wheel: {:?}{:?}", delta_x,delta_y);
        },
        _ => {}
    }
}

use std::{io, sync::mpsc::sync_channel};
use super::keymap;

pub fn test_listen_keyboard() {
    let (sync_sender, key_rx) = sync_channel(1024);
    thread::spawn(move || {
        listen_keyboard(sync_sender).unwrap();
    });
    // let key_map = keymap::get_keymap().unwrap();
    loop {
        match key_rx.recv() {
            Ok(key) => {
                // let key_str = format!("{:?}", key);
                // let value = key_map.get(&key_str.to_lowercase()).cloned().unwrap_or_default();
                // println!("key:{:?},value:{:?}", key_str,value);
                println!("{:?}", key);
            }
            Err(e) => {
                eprintln!("Error reading from serial port: {}", e);
                break;
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::{io, sync::mpsc::sync_channel};

    #[test]
    fn test_listen_keyboard() {
        let (sync_sender, key_rx) = sync_channel(1024);
        listen_keyboard(sync_sender).unwrap();
        loop {
            match key_rx.recv() {
                Ok(key) => {
                    println!("{:?}", key);
                }
                Err(e) => {
                    eprintln!("Error reading from serial port: {}", e);
                    break;
                }
            }
        }
    }
}