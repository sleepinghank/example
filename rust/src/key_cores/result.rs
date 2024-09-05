use std::{collections::HashMap, fs::File};
use std::io::Write;
use anyhow::Result;

// 保存结果文件名称
pub const RESULT_FILE: &str = "result.txt";

pub fn save_result(result_map: HashMap<u16, String>) -> Result<()> {
    let mut max_row = 0;
    let mut max_col = 0;
    // 1.循环result_map 中的key value
    for (key, value) in &result_map {
        // 2.key 的高8位为行号，低8位为列号
        let row = (key >> 8) as u8;
        max_row = max_row.max(row);
        let col = (key & 0xff) as u8;
        max_col = max_col.max(col);
        
    }
    // 3.创建二维数组
    let mut result = vec![vec!["".to_string(); max_col as usize + 1]; max_row as usize + 1];
    // 4.循环result_map 中的key value
    for (key, value) in &result_map {
        // 5.key 的高8位为行号，低8位为列号
        let row = (key >> 8) as u8;
        let col = (key & 0xff) as u8;
        // 6.将value赋值给二维数组
        result[row as usize][col as usize] = value.clone();
    }
    // 7.将数组输出到文件
    let mut file = File::create(RESULT_FILE)?;

    writeln!(file, "{{")?;
    for row in result.into_iter() {
        for (i, child) in row.iter().enumerate() {
            let item = child.clone();
            let item = item;
            // if let value = item {
            //     write!(file, "{}", value)?;
            // } else {
            //     write!(file, "0x00")?;
            // }
            if item.is_empty() {
                write!(file, "0x00")?;
            } else {
                write!(file, "{}", item.clone())?;
            }
            if i < row.len() - 1 {
                write!(file, ", ")?;
            }
        }
        writeln!(file, ",")?;
    }
    writeln!(file, "}}")?;
    Ok(())
}