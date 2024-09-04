use num_enum::TryFromPrimitive;
use serde::{Deserialize, Serialize};

// 将枚举（Enum）与 JSON 之间进行序列化和反序列化，并且希望枚举的值在 JSON 中以数字的形式表示，而不是字符串。
#[derive(Serialize, Deserialize, Debug, Clone,Copy,Eq, PartialEq, TryFromPrimitive)]
#[repr(u8)]
pub enum  ShortcutType {
    #[serde(rename = "1")]
    System  = 1,
    #[serde(rename = "2")]
    Custom  = 2,
}

impl Default for  ShortcutType {
    fn default() -> Self {
        Self ::System
    }
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct Shortcut {
    pub id: i32,
    pub name: String,
    pub keys: String,
    pub shortcut_type: ShortcutType,
}

// 测试用例
#[cfg(test)]
mod tests {
    use super::*;
    use std::convert::TryFrom;

    #[test]
    fn test_shortcut_type() {

        let custom = ShortcutType::default();
        let shortcut = Shortcut {
            id: 1,
            name: "test".to_string(),
            keys: "ctrl+shift+alt+1".to_string(),
            shortcut_type: custom,
        };
        println!("{:?}", serde_json::to_string(&shortcut).unwrap());
    }
}