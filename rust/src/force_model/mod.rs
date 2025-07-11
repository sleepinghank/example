use serde::{Deserialize, Serialize};

pub mod uart_data;
pub(crate) mod preprocess_data;

#[derive(Debug, PartialEq)]
enum ParseState {
    Sync,          // 等待帧头1
    HeaderSecond,  // 已收0xAA，等待0xBB
    Length,        // 等待长度
    Data(u8),      // 接收数据 (剩余字节数)
    Checksum,      // 等待校验和
}

pub struct FrameParser {
    state: ParseState,
    buffer: [u8; 10],  // 数据缓冲区 (10字节)
    data_len: u8,      // 预期数据长度
    index: usize,      // 当前存储位置
}

impl FrameParser {
    pub fn new() -> Self {
        FrameParser {
            state: ParseState::Sync,
            buffer: [0; 10],
            data_len: 0,
            index: 0,
        }
    }

    /// 处理单个字节的拆包逻辑
    /// 输入: 串口接收的原始字节
    /// 输出: Option<[u8; 10]> - 解析成功返回数据，否则返回None
    pub fn parse_byte(&mut self, byte: u8) -> Option<[u8; 10]> {
        match self.state {
            ParseState::Sync => {
                if byte == 0xAA {
                    self.state = ParseState::HeaderSecond;
                }
                None
            }
            ParseState::HeaderSecond => {
                if byte == 0xBB {
                    self.state = ParseState::Length;
                } else {
                    self.state = ParseState::Sync; // 同步失败
                }
                None
            }
            ParseState::Length => {
                // 只接受长度为10的数据包
                if byte == 10 {
                    self.data_len = byte;
                    self.index = 0;
                    self.state = ParseState::Data(10);
                } else {
                    self.state = ParseState::Sync; // 长度错误
                }
                None
            }
            ParseState::Data(remaining) => {
                self.buffer[self.index] = byte;
                self.index += 1;

                if remaining > 1 {
                    self.state = ParseState::Data(remaining - 1);
                    None
                } else {
                    self.state = ParseState::Checksum;
                    None
                }
            }
            ParseState::Checksum => {
                // 计算校验和 (长度+数据)
                let mut calc_checksum = self.data_len;  // 长度字段
                for b in &self.buffer {
                    calc_checksum ^= b;
                }

                // 返回结果或重置
                let result = if calc_checksum == byte {
                    Some(self.buffer)
                } else {
                    None
                };

                // 无论成功与否都重置解析器
                self.state = ParseState::Sync;
                result
            }
        }
    }
}


#[derive(Default,Debug, Clone,Serialize,Deserialize)]
#[repr(packed)]
pub struct RawData{
    pub x:u16,
    pub y:u16,
    pub size:u16,
    pub force_l:i16,
    pub force_r:i16,
}

fn deserialize_raw<T: Sized>(mut val: Vec<u8>) -> T {
    let t_len = ::core::mem::size_of::<T>();
    if val.len() < t_len {
        val.append(&mut vec![0u8;t_len - val.len()]);
    }
    unsafe { std::ptr::read(val.as_ptr() as *const _) }
}
/// 将任意类型转换为u8数组
fn serialize_raw<T: Sized>(val: &T) -> &[u8] {
    unsafe {
        ::core::slice::from_raw_parts(
            (val as *const T) as *const u8,
            ::core::mem::size_of::<T>(),)
    }
}
impl Into<Vec<u8>> for RawData {
    fn into(self) -> Vec<u8> {
        serialize_raw(&self).to_vec()
    }
}

impl From<Vec<u8>> for RawData {
    fn from(data: Vec<u8>) -> Self {
        deserialize_raw(data)
    }
}

#[derive(Default,Debug, Clone,Serialize,Deserialize)]
pub struct TrainingData{
    pub raw_data:RawData,
    pub is_btn_down:bool,
}

impl TrainingData {
    pub fn new(raw_data:RawData,is_btn_down:bool) -> Self {
        TrainingData { raw_data, is_btn_down }
    }
}
