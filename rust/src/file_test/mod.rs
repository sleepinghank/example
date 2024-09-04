
// 读取文件，文件是以‘，’分割的，十六进制数字，我需要将整个文件的数字转换为 大小为4096 byte的数组，数组的每个元素是u8类型，然后输出每个数组。

use std::fs::File;
use std::io::{self, Read};

pub fn split_fw() -> io::Result<()> {
    let mut file = File::open("./resources/PIX_fw.txt")?;
    let mut contents = String::new();
    file.read_to_string(&mut contents)?;
    // 去除\r\n
    contents = contents.replace("\r\n", "");
    // println!("{:?}", contents);
    let hex_strings = contents.split(',');
    let mut buffer = Vec::new();
    let mut count = 0;
    for hex_str in hex_strings {
        buffer.push(hex_str);
        if buffer.len() == 4096 {
            println!("{}",format!("static uint8_t TOUCH_FW{}[] = {{", count));
            for i in 0..4096 {
                print!("{},", buffer[i]);
                if i % 16 == 0 {
                    println!("");
                }
            }
            buffer.clear(); // 清空buffer以便重新填充
            count += 1;
            println!("}};");
        }
 
    }

    // 处理文件末尾不足4096个元素的情况
    if !buffer.is_empty() {
        println!("{}",format!("static uint8_t TOUCH_FW{}[] = {{", count));
        for i in 0..buffer.len() {
            print!("{},", buffer[i]);
            if i % 16 == 0 {
                println!("");
            }
        }
        println!("}};");
    }
    println!("count:{}", count);
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_split_fw() {
        split_fw().unwrap();
        assert_eq!(1, 1);
    }
}