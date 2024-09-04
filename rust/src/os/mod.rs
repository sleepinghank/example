mod win;


use std::sync::Arc;
use lazy_static::lazy_static;
use tokio::sync::mpsc::Sender;
pub use win::hotkey;
pub use win::kill_process;

lazy_static! {
    pub static ref WINDOW_SENDER : Arc<tokio::sync::Mutex<Option<Sender<u8>>>> = Arc::new(tokio::sync::Mutex::new(None));
}