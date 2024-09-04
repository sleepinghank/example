
use std::io;

use crossterm::event::{self, Event, KeyCode, KeyEvent};
use anyhow::{anyhow, Result};

use std::sync::mpsc::SyncSender;

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
    match event_type {
        rdev::EventType::KeyPress(key) => {
            sender.send(key).unwrap();
        }
        _ => {}
    }
}

pub fn read_input() -> io::Result<KeyEvent> {
    loop {
        if let Event::Key(key_event) = event::read()?
        {
            return Ok(key_event);
        }
    }
}

pub fn read_char() -> io::Result<char> {
    loop {
        if let Event::Key(KeyEvent {
            code: KeyCode::Char(c),
            ..
        }) = event::read()?
        {
            return Ok(c);
        }
    }
}

pub fn read_line() -> io::Result<String> {
    let mut line = String::new();
    while let Event::Key(KeyEvent { code, .. }) = event::read()? {
        match code {
            KeyCode::Enter => {
                break;
            }
            KeyCode::Char(c) => {
                line.push(c);
            }
            _ => {}
        }
    }

    Ok(line)
}

pub fn test_read_command() -> Result<()>{
    // listen_keyboard();
    Ok(())
}