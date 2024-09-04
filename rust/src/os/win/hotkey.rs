use windows_hotkeys::keys::{ModKey, VKey};
use windows_hotkeys::{HotkeyManager, HotkeyManagerImpl};
use anyhow::Result;
use tokio::runtime;
use tokio::sync::{mpsc, Mutex};
use crate::os::WINDOW_SENDER;


pub  fn  test() -> Result<()>{
    let vkey = VKey::from_char(char::from(52))?;
    let mod_key = ModKey::from_keyname("CTRL")?;
    let mut hkm = HotkeyManager::<()>::new();
    hkm.set_no_repeat(true);


    let hotkey = hkm.register(vkey, &[mod_key], move || {
        println!("Hotkey pressed!");
        // futures::executor::block_on(async {
        //     if let Some(sender) = WINDOW_SENDER.lock().await.as_ref() {
        //         println!("send Hotkey pressed!");
        //         let _ = sender.send(1).await;
        //     }
        // });
    }).unwrap();
    // 开线程跑

    hkm.event_loop();
    Ok(())
}

// 编写单元测试
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_hotkey() {
        test();
    }
}