use digest::Digest;
use downloader::Downloader;
use sha2::Sha256;
use tokio::fs::File;
use tokio::io::{AsyncReadExt, BufReader};

// Define a custom progress reporter:
struct SimpleReporterPrivate {
    last_update: std::time::Instant,
    max_progress: Option<u64>,
    message: String,
}
struct SimpleReporter {
    private: std::sync::Mutex<Option<SimpleReporterPrivate>>,
}

impl SimpleReporter {
    fn create() -> std::sync::Arc<Self> {
        std::sync::Arc::new(Self {
            private: std::sync::Mutex::new(None),
        })
    }
}

impl downloader::progress::Reporter for SimpleReporter {
    fn setup(&self, max_progress: Option<u64>, message: &str) {
        let private = SimpleReporterPrivate {
            last_update: std::time::Instant::now(),
            max_progress,
            message: message.to_owned(),
        };

        let mut guard = self.private.lock().unwrap();
        *guard = Some(private);
    }

    fn progress(&self, current: u64) {
        if let Some(p) = self.private.lock().unwrap().as_mut() {
            let max_bytes = p.max_progress.unwrap_or(current);
            if p.last_update.elapsed().as_millis() >= 1000 {
                println!(
                    "test file: {} of {} bytes.progress:{}, [{}]",
                    current, max_bytes,current * 100 / max_bytes, p.message
                );
                p.last_update = std::time::Instant::now();
            }
        }
    }

    fn set_message(&self, message: &str) {
        println!("test file: Message changed to: {}", message);
    }

    fn done(&self) {
        let mut guard = self.private.lock().unwrap();
        *guard = None;
        println!("test file: [DONE]");
    }
}

pub async  fn download(folder: &std::path::Path) -> anyhow::Result<String> {
    let file_name = "llama.cpp.zip";
    let mut downloader = Downloader::builder()
        .download_folder(folder)
        .build()?;

    let dl = downloader::Download::new("http://printapi.licheng-tech.com/llama/llama.cpp.zip");

    #[cfg(not(feature = "tui"))]
    let dl = dl.progress(SimpleReporter::create()).file_name(std::path::Path::new(file_name));

    let dl = {
        use downloader::verify;
        fn decode_hex(s: &str) -> Result<Vec<u8>, std::num::ParseIntError> {
            (0..s.len())
                .step_by(2)
                .map(|i| u8::from_str_radix(&s[i..i + 2], 16))
                .collect()
        }
        dl.verify(verify::with_digest::<sha3::Sha3_256>(
            decode_hex("a0443abce92766eedeabca0650b1d7bea2ce3c49cd2858747e34881d8728c0f5").unwrap()
        ))
    };

    let result = downloader.async_download(&[dl]).await?;

    for r in result {
        match r {
            Err(e) => println!("Error: {}", e.to_string()),
            Ok(s) => println!("Success: {}", &s),
        };
    }
    Ok(file_name.to_string())
}

/// 计算文件的 SHA-256 校验和
pub async  fn compute_sha256_of_file<P: AsRef<std::path::Path>>(file_path: P) -> anyhow::Result<String> {
    let file = File::open(file_path).await?;
    let mut reader = BufReader::new(file);

    let mut hasher = Sha256::new();
    let mut buffer = [0; 1024*8]; // 缓冲区大小可以根据实际情况调整

    let mut cnt = 0;
    loop {
        let bytes_read = reader.read(&mut buffer).await?;
        if bytes_read == 0 {
            break;
        }
        hasher.update(&buffer[..bytes_read]);
        cnt += bytes_read;
    }

    let result = hasher.finalize();
    Ok(format!("{:x}", result))
}
