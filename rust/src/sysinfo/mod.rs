use std::env;
use std::io::{BufRead, BufReader};
use std::os::windows::process::CommandExt;
// use std::io::{BufRead, BufReader};
// use std::os::windows::process::CommandExt;
use std::path::PathBuf;
use std::process::Stdio;
use anyhow::anyhow;
use regex::Regex;
use rustacuda::device::Device;
use rustacuda::error::CudaResult;
use std::process::Command;
use tokio::sync::mpsc;
use uuid::Uuid;
use winreg::enums::HKEY_LOCAL_MACHINE;
use winreg::RegKey;

#[derive(Debug, Clone)]
pub struct CpuInfo {
    pub brand_modifier: u8, // 9
    pub generation: u32, // 13600
    pub model_number:String, // Intel(R) Core(TM) i9-13600KF
    pub  cpu_type:CpuType,
}

/// CPU 信息
#[derive(Debug, Clone)]
pub enum CpuType {
    Intel ,// 9 13600 Intel(R) Core(TM) i9-13600KF
    Amd , // 7 7840 AMD Ryzen 7 7840H
}

impl CpuInfo {
    /// 获取 CPU 信息
    pub fn get_cpu_info() -> anyhow::Result<Self>{
        // let output = Command::new("wmic")
        //     .args(&["cpu", "get", "Name"])
        //     .output()?;
        //
        // let stdout = String::from_utf8(output.stdout)?;
        // let stderr = String::from_utf8(output.stderr)?;
        //
        // if !stderr.is_empty() {
        //     println!("Error: {}", stderr);
        //     return Err(anyhow::anyhow!("Error: {}", stderr));
        // }
        //
        // println!("CPU Info: {}", stdout);

        let cpu_brand = Self::cpu_info()?;

        let re_intel = Regex::new(r"Intel\(R\) Core\(TM\) i(\d)-(\d{4})").unwrap();
        let re_amd = Regex::new(r"AMD Ryzen (\d) (\d{4})").unwrap();

        if let Some(captures) = re_intel.captures(cpu_brand.as_str()) {
            let brand_modifier = captures.get(1).map_or(0, |m| m.as_str().parse().unwrap_or_default());
            let generation = captures.get(2).map_or(0, |m| m.as_str().parse().unwrap_or_default());
            return Ok(Self{
                brand_modifier,
                generation,
                model_number: cpu_brand,
                cpu_type:CpuType::Intel,
            });
        } else if let Some(captures) = re_amd.captures(cpu_brand.as_str()) {
            let brand_modifier = captures.get(1).map_or(0, |m| m.as_str().parse().unwrap_or_default());
            let generation = captures.get(2).map_or(0, |m| m.as_str().parse().unwrap_or_default());
            return Ok(Self{
                brand_modifier,
                generation,
                model_number: cpu_brand,
                cpu_type:CpuType::Amd,
            });
        }

        Err(anyhow!("No CPU info found"))
    }
    pub fn get_cpu_core_count() -> usize {
        num_cpus::get_physical()
    }
    fn cpu_info() -> anyhow::Result<String>{
        use sysinfo::{System, RefreshKind, CpuRefreshKind};

        let s = System::new_with_specifics(
            RefreshKind::new().with_cpu(CpuRefreshKind::everything()),
        );
        if let Some(cpu) = s.cpus().first(){
            return Ok(cpu.brand().to_string());
        }
        Err(anyhow!("get cpu info failed"))
    }
}

#[derive(Debug, Clone)]
pub struct NvidiaInfo {
    generation: u32, // 例如：4060
    model_number: String, // 例如：NVIDIA GeForce RTX 4060 Laptop GPU
    is_detect_cuda: bool, // 例如：true
    uuid: Option<String>, // 例如：GPU-bb54c881-ee8c-a775-474e-51d595d901eb
}

impl NvidiaInfo {
    // 根据显卡名称（NVIDIA GeForce RTX 4060 Laptop GPU）创建 NvidiaInfo 实例
    pub fn new_by_name(name: &str) -> anyhow::Result<Self> {
        let re = Regex::new(r"NVIDIA GeForce RTX (\d{4})")?;
        if let Some(captures) = re.captures(name) {
            let generation = captures.get(1).map_or(0, |m| m.as_str().parse().unwrap_or_default());
            let is_detect_cuda = Self::detect_cuda();
            let uuid = Self::get_uuid(name).unwrap_or_default();
            return Ok(Self {
                generation,
                model_number: name.to_string(),
                is_detect_cuda,
                uuid: Some(uuid),
            });
        }
        Err(anyhow!("No match found"))
    }

    pub fn check_cuda(&mut self) {
        self.is_detect_cuda = Self::detect_cuda();
    }

    /// 检测是否安装了 CUDA
    fn detect_cuda() -> bool{
        if let Ok(cuda_path) = env::var("CUDA_PATH") {
            println!("CUDA Toolkit is installed at: {}", cuda_path);
            return true;
        } else {
            // 通过注册表获取 CUDA 版本信息
            let hklm = RegKey::predef(HKEY_LOCAL_MACHINE);
            let cuda_key_path = r"SOFTWARE\NVIDIA Corporation\GPU Computing Toolkit\CUDA";
            if let Ok(cuda_key) = hklm.open_subkey(cuda_key_path) {
                if let Ok(version) = cuda_key.get_value::<String, _>("FirstVersionInstalled") {
                    println!("CUDA Version {}", version);
                }
                return true;
            } else {
                println!("No CUDA versions found in the registry.");
            }
        }

        false
    }

    fn get_uuid(name: &str) -> anyhow::Result<String> {
        if Self::detect_cuda(){
            use rustacuda::prelude::*;
            rustacuda::init(CudaFlags::empty())?;
            let devices = Device::devices()?;
            for device in devices {
                if let Ok(result) = Self::get_id_by_cuda(name, device){
                    return Ok(result);
                }
            }
        } else {
        //     通过 nvidia-smi --query-gpu=name,uuid --format=csv,noheader,nounits 命令获取，输出为 NVIDIA GeForce RTX 4060 Laptop GPU, GPU-bb54c881-ee8c-a775-474e-51d595d901eb
            let output = Command::new("nvidia-smi")
                .args(&["--query-gpu=name,uuid", "--format=csv,noheader,nounits"])
                .output()?;
            let stdout = String::from_utf8(output.stdout)?;
            let stderr = String::from_utf8(output.stderr)?;
            if !stderr.is_empty() {
                println!("Error: {}", stderr);
                return Err(anyhow!("Error: {}", stderr));
            }
            let reader = BufReader::new(stdout.as_bytes());
            for line in reader.lines() {
                let line = line?;
                let parts: Vec<&str> = line.split(", ").collect();
                if parts.len() == 2 {
                    let name = parts[0];
                    let uuid = parts[1];
                    if name.contains(name) {
                        return Ok(uuid.to_string());
                    }
                }
            }
        }
        Err(anyhow!("No match found"))
    }

    fn get_id_by_cuda(name: &str, device: CudaResult<Device>) -> anyhow::Result<String>{
        let device_info = device?;
        let device_name = device_info.name()?;
        if device_name.contains(name) {
            let id = device_info.uuid()?;
            let uuid = Uuid::from_bytes(id);
            return Ok(format!("GPU-{}", uuid));
        }
        Err(anyhow!("No match found"))
    }
}
#[derive(Debug, Clone)]
pub struct AmdGpuInfo {
    generation: u32, // 例如：7900
    model_number: String, // 例如：AMD Radeon RX 7900 XTX
}

impl AmdGpuInfo{
    // 根据显卡名称（AMD Radeon RX 7900 XTX）创建 AmdGpuInfo 实例
    pub fn new_by_name(name: &str) -> anyhow::Result<Self> {
        let re = Regex::new(r"AMD Radeon RX (\d{4})")?;
        if let Some(captures) = re.captures(name) {
            let generation:u32 = captures.get(1).map_or(0, |m| m.as_str().parse().unwrap_or_default());
            return Ok(Self {
                generation,
                model_number: name.to_string(),
            });
        }
        Err(anyhow!("No match found"))
    }
}

/// CPU 信息
#[derive(Debug, Clone)]
pub enum GpuInfo {
    Nvidia (NvidiaInfo),
    Amd (AmdGpuInfo),
    Other
}

impl GpuInfo {

    /// 获取 GPU 信息
    pub fn get_gpu_info() -> anyhow::Result<Self> {
        let output = Command::new("wmic")
            .args(&["path", "Win32_VideoController", "get", "Name"])
            .output()
            .expect("Failed to execute command");

        let stdout = String::from_utf8(output.stdout).expect("stdout is not valid UTF-8");
        let stderr = String::from_utf8(output.stderr).expect("stderr is not valid UTF-8");

        if !stderr.is_empty() {
            println!("Error: {}", stderr);
            return Err(anyhow::anyhow!("Error: {}", stderr));
        }
        for line in stdout.lines() {
            // 两端去 \r \n 空格
            let name = line.to_string().trim().to_string();
            if name.contains("NVIDIA GeForce RTX") {
                let nvidia_info = NvidiaInfo::new_by_name(name.as_str())?;
                return Ok(Self::Nvidia(nvidia_info));
            } else if name.contains("AMD Radeon RX") {
                let amd_info = AmdGpuInfo::new_by_name(name.as_str())?;
                return Ok(Self::Amd(amd_info));
            }
        }
        Ok(Self::Other)
    }
}

/// 获取系统运行内存
pub fn get_running_memory() -> anyhow::Result<u64>{
    use sysinfo::System;
    let s = System::new_all();
    return Ok(s.total_memory()/1024/1024/1024);
}

pub async fn commd_kill_test() -> anyhow::Result<()>{
    let (tx, mut rx) = mpsc::channel(100);
    let mut path = PathBuf::from(r"D:\");
    path.push(r"llama.cpp\build\bin\Release\");
    let mut model_path = r"D:\llama.cpp-master\build\bin\Debug\MiniCPM3-4B.Q8_0.gguf".to_string();
    path.push("llama-server.exe");
    println!("开始调用llama path:{:?},arg:{:?}",path.as_os_str(),model_path);
    let host = "0.0.0.0";
    let port = "8080";
    let threads = "8";
    let threads_batch = "8";
    let ctx_size = "512";
    let n_predict = "100";
    let batch_size = "512";
    let ubatch_size = "256";

    let mut child = Command::new(path.as_os_str())
        .env("CUDA_VISIBLE_DEVICES", "GPU-bb54c881-ee8c-a775-474e-51d595d901eb")
        .arg("-m").arg(model_path) // 将模型路径作为一个整体参数
        .arg("--host").arg(host)
        .arg("--port").arg(port)
        .arg("--threads").arg(threads)
        .arg("--threads-batch").arg(threads_batch)
        .arg("--ctx-size").arg(ctx_size)
        .arg("--n-predict").arg(n_predict)
        .arg("--batch-size").arg(batch_size)
        .arg("--ubatch-size").arg(ubatch_size)
        .arg("--mlock")
        .arg("--flash-attn")
        .creation_flags(0x08000000).stdout(Stdio::piped())
        .spawn()?;
    tokio::spawn(async move {
        println!("子进程启动成功");
        // 发送信号给子进程
        tokio::time::sleep(std::time::Duration::from_secs(20)).await;
        child.kill().expect("Failed to kill child process");
        println!("子进程已经被杀死");
        tx.send(()).await.unwrap(); // 通知主程序可以退出

    });

    // 主程序等待关闭信号
    rx.recv().await.unwrap();
    println!("主程序退出");
    // println!("Main program exiting and restarting...");
    //
    // // 重新启动程序
    // let status = Command::new(std::env::args().next().unwrap())
    //     .status()
    //     .expect("Failed to restart program");
    //
    // if !status.success() {
    //     eprintln!("Failed to restart program");
    // }
    //
    // println!("Restarted successfully.");
    Ok(())
}


pub async fn open_llama() -> anyhow::Result<()>{
    // let mut path = SystemInfo::get_appdata_directory().unwrap_or(PathBuf::from(r".\"));
    let mut path = PathBuf::from(r"D:\");
    path.push(r"llama.cpp\build\bin\Release\");
    let mut model_path = r"D:\llama.cpp-master\build\bin\Debug\MiniCPM3-4B.Q8_0.gguf".to_string();
    path.push("llama-server.exe");
    println!("开始调用llama path:{:?},arg:{:?}",path.as_os_str(),model_path);
    let host = "0.0.0.0";
    let port = "8080";
    let threads = "8";
    let threads_batch = "8";
    let ctx_size = "512";
    let n_predict = "100";
    let batch_size = "512";
    let ubatch_size = "256";

    if let Err(e) = Command::new(path.as_os_str())
        .env("CUDA_VISIBLE_DEVICES", "GPU-bb54c881-ee8c-a775-474e-51d595d901eb")
        .arg("-m").arg(model_path) // 将模型路径作为一个整体参数
        .arg("--host").arg(host)
        .arg("--port").arg(port)
        .arg("--threads").arg(threads)
        .arg("--threads-batch").arg(threads_batch)
        .arg("--ctx-size").arg(ctx_size)
        .arg("--n-predict").arg(n_predict)
        .arg("--batch-size").arg(batch_size)
        .arg("--ubatch-size").arg(ubatch_size)
        .arg("--mlock")
        .arg("--flash-attn")
        // .creation_flags(0x08000000).stdout(Stdio::piped())
        .spawn(){
        return Err(anyhow!("调用llama失败 cause : {}",e));
    }
    Ok(())
}

pub fn get_nvidia_idx() -> anyhow::Result<u8>{
    use rustacuda::prelude::*;
    rustacuda::init(CudaFlags::empty())?;
    let num = Device::num_devices()?;
    println!("Number of devices: {}", num);
    let device = Device::get_device(0)?;
    let name = device.name()?;
    println!("Device Name: {}", name);
    let id = device.uuid()?;
    let uuid = Uuid::from_bytes(id);
    println!("Device UUID: {}", uuid);
    Ok(0)
}

/// 通过IPC活跃状态判断前端是否已经启动
pub async fn check_front_end(arg: String) -> anyhow::Result<()>{
    let app_path = "shell:appsFolder\\A1232126.InateckOrigin_1tk2hf84048yp!com.inateck.synergy";
    println!("微软store 调用前端路径 {:?}",app_path);
    match Command::new("cmd")
        .args(&["/c", "start", "", app_path]).arg(arg).output() {
        Ok(output) => {
            let stdout = String::from_utf8(output.stdout).expect("stdout is not valid UTF-8");
            let stderr = String::from_utf8(output.stderr).expect("stderr is not valid UTF-8");
            if !stderr.is_empty() {
                println!("Error: {}", stderr);
                return Err(anyhow::anyhow!("Error: {}", stderr));
            }

        },
        Err(e) => {
            return Err(anyhow!("调用前端失败 cause : {}",e));
        }
    }

    Ok(())
}

pub async fn check_env() -> anyhow::Result<()>{
    loop {
        if let Ok(env_path) = env::var("TEST_PATH") {
            println!("TEST_PATH is installed at: {}", env_path);
        } else {
            println!("No TEST_PATH found in the environment.");
        }
        tokio::time::sleep(std::time::Duration::from_secs(1)).await;
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use sysinfo::Cpu;
    use super::*;

    #[test]
    fn test_nvidia_info() {
        let nvidia_info = NvidiaInfo::new_by_name("NVIDIA GeForce RTX 4060 Laptop GPU").unwrap();
        println!("nvidia_info: {:?}", nvidia_info);
    }

    #[test]
    fn test_get_gpu_info() {
        let gpu_info = GpuInfo::get_gpu_info().unwrap();
        println!("gpu_info: {:?}", gpu_info);
    }

    #[test]
    fn test_get_running_memory(){
        let memory = get_running_memory().unwrap();
        println!("memory: {:?}", memory);
    }


    #[test]
    fn test_get_cpu_info(){
        let cpu = CpuInfo::get_cpu_info().unwrap();
        println!("cpu: {:?}", cpu);
    }

    #[test]
    fn test_detect_cuda(){
        let result = NvidiaInfo::detect_cuda();
        println!("detect_cuda: {:?}", result);
    }
}
