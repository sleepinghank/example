use windows_hotkeys::keys::{ModKey, VKey};
use windows_hotkeys::{HotkeyManager, HotkeyManagerImpl};

pub fn test() {
    let mut hkm = HotkeyManager::new();

    hkm.register(VKey::A, &[ModKey::Alt], || {
        println!("Hotkey ALT + A was pressed");
    })
    .unwrap();

    hkm.event_loop();
}