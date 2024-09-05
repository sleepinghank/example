

use std::env;
use tokio::process::Command;
use tokio::sync::mpsc;
use tokio::time::sleep;
use crate::os::WINDOW_SENDER;

mod port;
mod touch;
mod kalman;
mod encrypt;
mod os;
mod openai;
mod file_test;
mod serial; 
mod key_cores;


pub trait Animal {
    fn make_noise(&self) -> String;
}

pub struct Dog;
pub struct Cat;

impl Animal for Dog {
    fn make_noise(&self) -> String {
        "Woof!".to_string()
    }
}

impl Animal for Cat {
    fn make_noise(&self) -> String {
        "Meow!".to_string()
    }
}

// 假设有一个简单的对话记录结构体
struct Dialogue {
    speaker: String,
    message: String,
}

// 实现一个函数，将对话记录转换为特定的字符串格式
fn format_dialogues(dialogues: Vec<Dialogue>) -> Option<String> {
    if dialogues.is_empty() {
        return None;
    }
    let mut formatted_string = "[".to_owned();
    for dialogue in dialogues {
        // 对每个对话进行格式化，并追加到结果字符串中
        formatted_string.push_str(&format!(
            "(\"{}\", \"{}\"),",
            dialogue.speaker, dialogue.message
        ));
    }
    // 移除最后一个逗号
    formatted_string.pop();
    formatted_string.push(']');
    Some(formatted_string)
}

use rdev::{listen, Event, EventType};

fn callback(event: Event) {
    match event.event_type {
        EventType::MouseMove { x, y } => {
            println!("Mouse moved to: ({}, {})", x, y);
        }
        EventType::ButtonPress(button) => {
            println!("Mouse button {:?} pressed", button);
        }
        EventType::ButtonRelease(button) => {
            println!("Mouse button {:?} released", button);
        }
        _ => (),
    }


    
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // key_cores::serial::test_read_from_serial_port();
    // key_cores::cli::test_listen_keyboard();
    key_cores::start().unwrap();

    Ok(())
}


