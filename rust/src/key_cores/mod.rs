

/// 串口通信模块
pub mod serial;
/// 命令行交互模块
pub mod cli;
/// 按键映射模块
pub mod keymap;
/// 结果保存模块
pub mod result;

use std::collections::HashMap;
use std::fmt::format;
use std::thread::{self, spawn};
use std::{sync::mpsc};
use crossterm::event;
use anyhow::Result;
use dialoguer::{theme::ColorfulTheme, Select,Input};
use regex::Regex;
use rdev::listen as listen_event;
use std::sync::mpsc::sync_channel;
use serde::{Deserialize, Serialize};
use std::io;
use std::io::Write;
use console::Term;

/// 构建命令行状态机
#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash,Serialize, Deserialize)]
enum State {
    // 等待串口输入坐标
    WaitSerialInput,
    // 等待用户输入字符
    WaitUserInput,
    // 输入第一个字符
    FirstInput,
    // 进入字符串输入模式
    StringInput,
    // 结束退出
    Exit,
}

impl Default for State {
    fn default() -> Self {
        State::WaitSerialInput
    }
}

struct KeyResult {
    /// 当前状态
    state: State,
    /// 串口输入的坐标
    serial_input: Option<(u8, u8)>, // row, col
    /// 用户输入的字符
    user_input: Option<String>,
    /// 字符串输入的缓冲区
    string_buffer: String,
    serial_rx: mpsc::Receiver<String>,
    key_rx: mpsc::Receiver<rdev::Key>,
    // 保存结果的map
    result_map: HashMap<u16, String>,
    key_map: HashMap<String, String>,
    term: Term,
}

impl KeyResult {
    fn new(port_name: String,baud_rate: u32,) -> Result<Self> {
        let (serial_tx, serial_rx) = mpsc::channel();
        // 开启线程读取串口数据
        thread::spawn(move || {
            serial::read_from_serial_port(port_name.as_str(), baud_rate, serial_tx).unwrap();
        });
        let key_map = keymap::get_keymap()?;
        println!("{:?}", key_map);
        let (sync_sender, key_rx) = sync_channel(1024);
        thread::spawn(move || {
            cli::listen_keyboard(sync_sender).unwrap();
        });
        // 开启线程监听键盘
        Ok(KeyResult {
            state: State::WaitSerialInput,
            serial_input: None,
            user_input: None,
            string_buffer: "".to_string(),
            result_map: HashMap::new(),
            serial_rx,
            key_rx,
            key_map,
            term: Term::stderr(),
        })
    }

    fn run(&mut self) -> Result<()>{
        loop {
            unwrapped_output(format!("{:?}",self.state).as_str());
            match self.state {
                State::WaitSerialInput => self.wait_serial_input()?,
                State::WaitUserInput => self.wait_user_input()?,
                State::FirstInput => self.first_input()?,
                State::StringInput => self.string_input()?,
                State::Exit =>  self.exit()?,
            }
        }
    }

    // 等待串口输入坐标
    fn wait_serial_input(&mut self) -> Result<()>{
        // 1.监听serial_rx 串口输入，当捕获到坐标时，进入下一个状态
        // 2.监听cli_rx 用户输入，当捕获到ESC时，退出
        loop {
            match self.serial_rx.recv() {
                Ok(data) => {
                    println!("{:?}", data);
                   // 利用正则匹配“downkey:6,4” 这样的字符串，提取坐标 6 为 row 4 为 col
                    let re = Regex::new(r"downkey:(\d+),(\d+)").unwrap();
                    match re.captures(&data) {
                        Some(caps) => {
                            let row = caps.get(1).unwrap().as_str().parse::<u8>().unwrap_or(99);
                            let col_num = caps.get(2).unwrap().as_str().parse::<u8>().unwrap_or(99);
                            // println!("row: {}, col: {}", row, col);
                            // 判断col 是否为2的几次方
                            if col_num.count_ones() != 1 {
                                continue;
                            }
                            let col = col_num.trailing_zeros() as u8;
                            self.serial_input = Some((row, col));
                            self.state = State::WaitUserInput;
                            break;
                        },
                        None => {
                            
                        }
                    }
                },
                Err(e) => {
                    // eprintln!("Error reading from serial port: {}", e);
                    // break;
                    continue;
                }
            }
        }
        while let Ok(_) = self.key_rx.try_recv() {}
        while let Ok(_) = self.serial_rx.try_recv() {}
        Ok(())
    }

    fn wait_user_input(&mut self) -> Result<()>{
        if self.serial_input.is_none() {
            self.state = State::WaitSerialInput;
            return Ok(());
        }
        unwrapped_output(format!("Row:{},Col:{}. Please press a key on the keyboard:", self.serial_input.unwrap().0, self.serial_input.unwrap().1).as_str());
        loop {
            match self.key_rx.recv() {
                Ok(key) => {
                    if let Some(key_str) = self.user_input.as_ref() {
                        if key == rdev::Key::Return {
                            self.result_map.insert((self.serial_input.unwrap().0 as u16) << 8 | self.serial_input.unwrap().1 as u16, self.string_buffer.clone());
                            self.save_result()?;
                            self.user_input = None;
                            self.string_buffer.clear();
                            self.state = State::WaitSerialInput;
                            self.term.read_line()?;
                            while let Ok(_) = self.key_rx.try_recv() {}
                            while let Ok(_) = self.serial_rx.try_recv() {}
                            return Ok(());
                        }
                        if key == rdev::Key::KeyI {
                            if key_str == "KeyI" {
                                self.user_input = None;
                                self.string_buffer.clear();
                                self.state = State::StringInput;
                                println!();
                                return Ok(());
                            }
                        }
                    }
                    // 将 key 转换为字符串 在key_map中查找对应的值。如果找不到就展示原有值
                    let key_str = format!("{:?}", key);
                    let value = self.key_map.get(&key_str.to_lowercase()).unwrap_or(&key_str);
                    // println!("Row:{},Col:{}. KeyVal:{}", self.serial_input.unwrap().0, self.serial_input.unwrap().1,value);
                    unwrapped_output(&value);
                    self.user_input = Some(key_str.clone());
                    self.string_buffer = value.to_string();
                },
                Err(e) => continue,
            }
            
        }
        Ok(())
    }
    fn first_input(&mut self) -> Result<()>{

        Ok(())
    }
    fn string_input(&mut self) -> Result<()>{
        if self.serial_input.is_none() {
            self.state = State::WaitSerialInput;
            return Ok(());
        }
        // 获取用户输入
        let input: String = Input::with_theme(&ColorfulTheme::default())
        .with_prompt(format!("Row:{},Col:{}. KeyVal:", self.serial_input.unwrap().0, self.serial_input.unwrap().1))
        .interact_text_on(&self.term)?;
        self.result_map.insert((self.serial_input.unwrap().0 as u16) << 8 | self.serial_input.unwrap().1 as u16, input);
        self.save_result()?;
        self.string_buffer.clear();
        while let Ok(_) = self.key_rx.try_recv() {}
        while let Ok(_) = self.serial_rx.try_recv() {}
        self.state = State::WaitSerialInput;
        Ok(())
    }
    fn exit(&mut self) -> Result<()>{
        Ok(())
    }

    fn save_result(&self) -> Result<()>{
        // 保存结果
        result::save_result(self.result_map.clone())?;
        Ok(())
    }
}

fn unwrapped_output(out: &str) {
    print!("{},", out);
    // 刷新标准输出
    if io::stdout().flush().is_err() {
        println!("flush err")
    }
}


const SERIAL_PORT_BAUD_RATES: [u32; 12] = [1200  ,2400  ,4800  ,9600  ,14400 ,19200 ,38400 ,57600 ,115200,230400,460800,921600];

pub fn start() -> Result<()> {
    // 1.获取所有可用串口
    let mut ports = serial::get_available_serialport()?;
    ports.reverse();
    // 2.选择一个串口
    let select_port = Select::with_theme(&ColorfulTheme::default())
        .with_prompt("Please select your serialport")
        .default(0)
        .items(&ports[..])
        .interact()?;
    // 3.选择一个波特率
    let select_rate = Select::with_theme(&ColorfulTheme::default())
    .with_prompt("Please select baud rate")
    .default(8)
    .items(&SERIAL_PORT_BAUD_RATES[..])
    .interact()?;
    // 4.开始输入监听
    let mut key_result = KeyResult::new(ports[select_port].clone(),SERIAL_PORT_BAUD_RATES[select_rate])?;
    // 5.运行状态机
    key_result.run()?;
    Ok(())
}