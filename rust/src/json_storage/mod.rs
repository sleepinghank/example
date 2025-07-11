use serde::{Deserialize, Serialize};
use std::fs;
use std::path::Path;
use std::io;
use anyhow::{Result, Context};
use chrono::{DateTime, Utc, Local};
use rand::Rng;

/// JSON存储管理器
pub struct JsonStorage {
    file_path: String,
}

impl JsonStorage {
    /// 创建新的JSON存储管理器
    pub fn new(file_path: &str) -> Self {
        Self {
            file_path: file_path.to_string(),
        }
    }

    /// 保存对象到JSON文件
    pub fn save<T: Serialize>(&self, data: &T) -> Result<()> {
        let json_string = serde_json::to_string_pretty(data)
            .context("序列化对象到JSON失败")?;
        
        // 确保目录存在
        if let Some(parent) = Path::new(&self.file_path).parent() {
            fs::create_dir_all(parent)
                .context("创建目录失败")?;
        }
        
        fs::write(&self.file_path, json_string)
            .context("写入JSON文件失败")?;
        
        Ok(())
    }

    /// 从JSON文件加载对象
    pub fn load<T: for<'de> Deserialize<'de>>(&self) -> Result<T> {
        if !Path::new(&self.file_path).exists() {
            return Err(anyhow::anyhow!("文件不存在: {}", self.file_path));
        }
        
        let content = fs::read_to_string(&self.file_path)
            .context("读取JSON文件失败")?;
        
        let data: T = serde_json::from_str(&content)
            .context("反序列化JSON失败")?;
        
        Ok(data)
    }

    /// 初始化新文件（如果不存在）
    pub fn init_file<T: Serialize + Default>(&self) -> Result<()> {
        if !Path::new(&self.file_path).exists() {
            let default_data = T::default();
            self.save(&default_data)?;
            println!("已初始化新文件: {}", self.file_path);
        }
        Ok(())
    }

    /// 检查文件是否存在
    pub fn exists(&self) -> bool {
        Path::new(&self.file_path).exists()
    }

    /// 删除文件
    pub fn delete(&self) -> Result<()> {
        if self.exists() {
            fs::remove_file(&self.file_path)
                .context("删除文件失败")?;
        }
        Ok(())
    }

    /// 获取文件路径
    pub fn get_file_path(&self) -> &str {
        &self.file_path
    }

    /// 生成临时文件夹路径（基于年月日时分秒和随机数）
    pub fn generate_temp_path(base_dir: &str, prefix: &str, extension: &str) -> String {
        let now = Local::now();
        let timestamp = now.format("%Y%m%d_%H%M%S_%3f").to_string(); // 年月日_时分秒_毫秒
        
        let mut rng = rand::thread_rng();
        let random_num: u32 = rng.gen();
        
        format!("{}/{}_{}_{}.{}", base_dir, prefix, timestamp, random_num, extension)
    }

    /// 创建临时存储管理器
    pub fn new_temp(base_dir: &str, prefix: &str, extension: &str) -> Self {
        let file_path = Self::generate_temp_path(base_dir, prefix, extension);
        Self { file_path }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_json_storage() {

    }
} 