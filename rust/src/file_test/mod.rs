
// 读取文件，文件是以‘，’分割的，十六进制数字，我需要将整个文件的数字转换为 大小为4096 byte的数组，数组的每个元素是u8类型，然后输出每个数组。

use std::fs::File;
use std::io::{self, Read};
use std::path::Path;

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


fn file_save() -> anyhow::Result<()> {
    let data_path = Path::new(r"C:\Users\hank\Downloads\TP_12581-2860-uptpdata.bin");
    if data_path.exists() {
        println!("file exists, size:{}", data_path.metadata()?.len());
        let mut file = File::open(data_path)?;
        let mut buffer = Vec::new();
        file.read_to_end(&mut buffer)?;
        println!("file size:{}", buffer.len());
    } else {
        println!("file not exists");
    }
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

    #[test]
    fn test_file_save() {
        file_save().unwrap();
        assert_eq!(1, 1);
    }
}