
use std::{ptr, time::SystemTime};
use winapi::{
    shared::{minwindef::{BOOL, LPARAM, UINT, WPARAM}, windef::POINT},
    um::{
        processthreadsapi::GetCurrentThreadId,
        winuser::{
            AttachThreadInput, GetClassNameW, GetCursorPos, GetFocus, GetForegroundWindow, GetWindow, GetWindowLongPtrW, GetWindowRect, GetWindowTextLengthW, GetWindowTextW, GetWindowThreadProcessId, SendMessageW, EM_GETSEL, WM_LBUTTONDOWN, WS_CHILD
        },
    },
};
use winapi::shared::windef::HWND;
use winapi::um::winuser::GWL_STYLE;
use rdev::{listen as listen_event, Event, EventType};
use std::{sync::mpsc::SyncSender, thread};
use get_selected_text::get_selected_text;
pub fn get_selected() -> anyhow::Result<String>{
    let hand = remote_get_focus();
    if let Some(hwnd) = hand {
        let title = get_active_window_title(hwnd)?;
        println!("title: {}", title);
        if is_text_selected(hwnd) {
            let text = get_selected_text().map_err(|e| anyhow::anyhow!("获取选中文本失败: {:?}", e))?;
            return Ok(text);
        }
    }
    Err(anyhow::anyhow!("未选中文本"))
}


// 定义一个调试输出函数
fn output_debug_printf(msg: &str) {
    println!("{}", msg);
}

/// 获取当前拥有焦点的窗口句柄
fn remote_get_focus() -> Option<HWND> {
    let foreground_window = unsafe { GetForegroundWindow() };
    if foreground_window.is_null() {
        output_debug_printf("无法获取当前激活窗口的句柄");
        return None;
    }

    let remote_thread_id = unsafe { GetWindowThreadProcessId(foreground_window, ptr::null_mut()) };
    let current_thread_id = unsafe { GetCurrentThreadId() };

    if unsafe { AttachThreadInput(remote_thread_id, current_thread_id, true as BOOL) } == 0 {
        output_debug_printf("无法附加线程输入");
        return None;
    }

    let focused_window = unsafe { GetFocus() };
    if unsafe { AttachThreadInput(remote_thread_id, current_thread_id, false as BOOL) } == 0 {
        output_debug_printf("无法分离线程输入");
    }

    Some(focused_window)
}

/// 检查是否为文本选择状态
fn is_text_selected(hwnd: HWND) -> bool {
    let style = unsafe { GetWindowLongPtrW(hwnd, GWL_STYLE) };
    if ((style as u32) & WS_CHILD) == 0 {
        return false;
    }

    let mut class_name = [0u16; 256];
    unsafe { GetClassNameW(hwnd, class_name.as_mut_ptr(), class_name.len() as i32) };
    let class_name_str = String::from_utf16_lossy(&class_name);
    output_debug_printf(&format!("窗口类名: {}", class_name_str));

    let mut sel_start: isize = 0;
    let sel_end: isize = unsafe { SendMessageW(hwnd, EM_GETSEL as UINT, sel_start as WPARAM, 0) };
    if sel_start != sel_end {
        output_debug_printf(&format!("选中文本长度: {}", sel_end - sel_start));
        true
    } else {
        false
    }
}

/// 获取当前激活窗口的标题
fn get_active_window_title(hwnd: HWND) -> anyhow::Result<String> {
    if hwnd.is_null() {
        return Err(anyhow::anyhow!("无法获取当前激活窗口的句柄"));
    }

    let n_length = unsafe { GetWindowTextLengthW(hwnd) };
    if n_length == 0 {
        return Err(anyhow::anyhow!("无法获取当前激活窗口的标题"));
    }
    
    let mut title = vec![0u16; n_length as usize];
    unsafe { GetWindowTextW(hwnd, title.as_mut_ptr(), n_length as i32 + 1) };
    Ok(String::from_utf16_lossy(&title))
}

pub fn listen_mouse(sender: SyncSender<(i32,i32)>) -> anyhow::Result<()>{
    if let Err(err) = listen_event(move |event: rdev::Event| {
        callback(event,sender.clone());
    }) {
        return Err(anyhow::anyhow!("Error: {:?}", err));
    }
    Ok(())
}

// 全局变量声明
static mut G_MOUSE_DOWN: bool = false;
static mut G_MOUSE_MOVE: bool = false;
static mut G_DOUBLE_CLICK: bool = false;
static mut G_LAST_CLICK_TIME: u64 = 0;
static mut G_CURRENT_CLICK_TIME: u64 = 0;
static mut G_LAST_CLICK_POS: POINT = POINT { x: 0, y: 0 };
static mut G_CURRENT_CLICK_POS: POINT = POINT { x: 0, y: 0 };

// 获取当前时间戳
fn now() -> u64 {
    SystemTime::now()
        .duration_since(SystemTime::UNIX_EPOCH)
        .expect("Time went backwards")
        .as_secs()
}

// 获取鼠标当前位置
fn get_mouse_position() -> POINT {
    let mut point = POINT { x: 0, y: 0 };
    unsafe {
        GetCursorPos(&mut point);
    }
    point
}

// 检查是否为双击
fn is_double_click(current_time: u64, current_pos: POINT) -> bool {
    let last_click_time = unsafe { G_LAST_CLICK_TIME };
    let last_click_pos = unsafe { G_LAST_CLICK_POS };

    if current_time - last_click_time <= 250 && current_pos.x == last_click_pos.x && current_pos.y == last_click_pos.y {
        true
    } else {
        false
    }
}

// 更新全局变量
fn update_globals(current_time: u64, current_pos: POINT) {
    unsafe {
        G_LAST_CLICK_TIME = G_CURRENT_CLICK_TIME;
        G_LAST_CLICK_POS = G_CURRENT_CLICK_POS;
        G_CURRENT_CLICK_TIME = current_time;
        G_CURRENT_CLICK_POS = current_pos;
    }
}


// 监听鼠标事件
fn callback(event: Event,sender: SyncSender<(i32,i32)>) {
    let event_type = event.event_type;
    let time = event.time;

    match event_type {
        EventType::ButtonPress(b) => {
            if b == rdev::Button::Left {
                let current_time = now();
                let current_pos = get_mouse_position();

                unsafe {
                    G_MOUSE_DOWN = true;
                }

                update_globals(current_time, current_pos);

                if is_double_click(current_time, current_pos) {
                    unsafe {
                        G_DOUBLE_CLICK = true;
                    }
                } else {
                    unsafe {
                        G_DOUBLE_CLICK = false;
                    }
                }
            }
        }
        EventType::ButtonRelease(b) => {
            if b == rdev::Button::Left {
                unsafe {
                    G_MOUSE_DOWN = false;
                }
                let p = get_mouse_position();
                if unsafe { G_MOUSE_MOVE } {
                    unsafe {
                        G_MOUSE_MOVE = false;
                    }
                    sender.send((p.x,p.y)).unwrap();
                } else if unsafe { G_DOUBLE_CLICK } {
                    unsafe {
                        G_DOUBLE_CLICK = false;
                    }
                    sender.send((p.x,p.y)).unwrap();
                }
            }
        }
        EventType::MouseMove { x, y } => {
            if unsafe { G_MOUSE_DOWN } {
                unsafe {
                    G_MOUSE_MOVE = true;
                }
            }
        }
        _ => {}
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_remote_get_focus() {
        let hwnd = remote_get_focus();
        println!("hwnd: {:?}", hwnd);
    }

    #[test]
    fn test_is_text_selected() {
        let hwnd = remote_get_focus().unwrap();
        let is_selected = is_text_selected(hwnd);
        println!("is_selected: {:?}", is_selected);
    }

    #[test]
    fn test_get_active_window_title() {
        let hwnd = remote_get_focus().unwrap();
        let title = get_active_window_title(hwnd).unwrap();
        println!("title: {:?}", title);
    }
}