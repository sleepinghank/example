use std::sync::atomic::AtomicBool;
use lazy_static::lazy_static;

//系统事件发送器|蓝牙事件，网络变化事件等
lazy_static! {
    pub static ref SYS_KEY_STATUS: AtomicBool = AtomicBool::new(false); // 记录按键状态
    pub static ref SYS_KEY_QUIT: AtomicBool = AtomicBool::new(false); // 记录结束状态
}
