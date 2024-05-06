
/// This module contains structs and functions for interacting with a custom AI service.
/// It provides functionality for device registration, translation requests, and dialogue requests.

use reqwest::Response;
use serde::{Deserialize, Serialize};
use futures_util::StreamExt;
use bytes::BytesMut;
use tokio::sync::mpsc::UnboundedReceiver;
use anyhow::Result;

use super::crypto::encrypt_id;

/// Represents a device registration request.
#[derive(Serialize, Deserialize, Debug, Clone, Default)]
pub struct DeviceRegisterRequest {
    // Device UUID
    id: String,
    // Product information
    product_info: ProductInfo,
    // Device firmware version
    firmware_version: u32,
    // Desktop software version
    software_version: u32,
    // Encryption (to be determined)
    encrypt: String,
}

/// Represents product information.
#[derive(Serialize, Deserialize, Debug, Clone, Default)]
pub struct ProductInfo {
    // Product name
    device_name: String,
    // Product model code
    product_model_code: u32,
    // Product type (1: Keyboard, 2: Mouse, 3: Touchpad, 4: MulKeyboardTouchpad, 100: Other)
    device_type: u8,
}

/// Represents a translation request.
#[derive(Serialize, Deserialize, Debug, Clone, Default)]
pub struct TranslationRequest {
    // Device UUID
    id: String,
    // UUID, session id
    session_id: String,
    // Target language for translation
    target_lang: String,
    // Source text to be translated
    source_text: String,
    // Token for encryption (to be determined)
    token: String,
}

/// Represents a dialogue request.
#[derive(Serialize, Deserialize, Debug, Clone, Default)]
pub struct DialogueRequest {
    // Device UUID
    id: String,
    // Question content
    content: String,
}

/// Represents a device registration response.
#[derive(Serialize, Deserialize, Debug, Clone, Default)]
pub struct DeviceRegisterResponse {
    // Data
    data: Option<serde_json::Value>,
    // Message
    message: String,
    // Status
    status: i8,
}

/// Base URL for API requests
const API_BASE: &str = r"http://176.58.108.46:4567/";

/// Sends a device registration request to the API.
pub async fn device_register(id: &str, mut request: DeviceRegisterRequest) -> anyhow::Result<bool> {
    // Encrypt the device ID
    let (digest, base) = encrypt_id(id)?;
    request.id = base;
    request.encrypt = digest;

    let client = reqwest::Client::new();
    let response: DeviceRegisterResponse = client
        .post(format!("{}device_register", API_BASE))
        .json(&request)
        .send()
        .await?
        .json()
        .await?;
    if response.status < 0 {
        return Err(anyhow::anyhow!(response.message));
    }
    Ok(true)
}

/// 将响应流转换为字符串流
///
/// # Arguments
///
/// * `response` - 响应体
///
/// # Returns
///
/// 返回一个无边界接收器，用于接收响应流的数据。以[DONE]作为结束标志。
async fn  stream_response(response: Response) -> Result<UnboundedReceiver<String>> {

    let mut stream = response.bytes_stream();
    let mut buffer = BytesMut::with_capacity(20);
    let (tx, rx) = tokio::sync::mpsc::unbounded_channel();
    if let Some(result) = stream.next().await {
        match result {
            Ok(bytes) => {
                let data: String = String::from_utf8_lossy(&bytes).to_string();
                if bytes.len() == 16 {
                    if data.eq_ignore_ascii_case("Device not found") {
                        return Err(anyhow::anyhow!("Device not found"));
                    }
                }
                if let Err(e) = tx.send(data) {
                    return Err(anyhow::anyhow!("sender send error: {:?}", e));
                };
            }
            Err(err) => {
                return Err(anyhow::anyhow!("receiver translate error: {:?}", err));
            }
        }
    }

    tokio::spawn(async move {
        while let Some(result) = stream.next().await {
            match result {
                Ok(bytes) => {
                    let size = bytes.len();
                    if size == 0 {
                        break;
                    }

                    buffer.extend_from_slice(&bytes);

                    if buffer.len() >= 16 {
                        let data: String = String::from_utf8_lossy(&buffer).to_string();
                        if let Err(e) = tx.send(data) {
                            println!("sender send error: {:?}", e);
                            break;
                        };
                        buffer.clear();
                    }
                }
                Err(err) => {
                    println!("receiver translate error: {:?}", err);
                    break;
                }
            }
        }

        if !buffer.is_empty() {
            if let Err(e) = tx.send(String::from_utf8_lossy(&buffer).to_string()) {
                println!("sender send error: {:?}", e);
                return;
            };
        };

        if let Err(e) = tx.send("[DONE]".to_string()) {
            println!("sender send error: {:?}", e);
        };
    });

    Ok(rx)
}

/// Sends a translation request to the API.
pub async fn translate(
    id: String,
    session_id: String,
    target_lang: String,
    source_text: String,
) -> anyhow::Result<UnboundedReceiver<String>> {
    let request = TranslationRequest {
        id,
        session_id,
        target_lang,
        source_text,
        token: "1".to_string(),
    };

    let client = reqwest::Client::new();
    let response = client
        .post(format!("{}translate", API_BASE))
        .json(&request)
        .send()
        .await?;

    let receiver = stream_response(response).await?;
    Ok(receiver)
}

/// Sends a chat request to the API.
pub async fn chat(id: String, content: String) -> anyhow::Result<UnboundedReceiver<String>> {
    let request = DialogueRequest { id, content };

    let client = reqwest::Client::new();
    let response = client
        .post(format!("{}chat", API_BASE))
        .json(&request)
        .send()
        .await?;

    let receiver = stream_response(response).await?;
    Ok(receiver)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::{stdout, Write};

    #[tokio::test]
    async fn test_device_register() {
        // Test device registration
        let id = "d5e80a2f-de3e-4b6f-951b-0250e455f329".to_string();
        let product_info = ProductInfo {
            device_name: "KB06103".to_string(),
            product_model_code: 1048657,
            device_type: 1,
        };
        let firmware_version = 265;
        let software_version = 0;
        let request = DeviceRegisterRequest {
            id: "".to_string(),
            product_info,
            firmware_version,
            software_version,
            encrypt: "encrypt".to_string(),
        };

        let result = device_register(id.as_str(), request).await;

        if let Err(e) = result {
            println!("{:?}", e);
            assert_eq!(1, 0);
        } else {
            println!("{:?}", result.unwrap());
            assert_eq!(1, 1);
        }
    }

    #[tokio::test]
    async fn test_translate() {
        // Test translation
        let id = "d5e80a2f-de3e-4b6f-951b-0250e455f329".to_string();
        let session_id = "d5e80a2f-de3e-4b6f-951b-0250e455f329".to_string();
        let target_lang = "English".to_string();
        let source_text = r"假如今天是我生命中的最后一天。".to_string();

        let mut lock = stdout().lock();

        match translate(id, session_id, target_lang, source_text).await {
            Ok(mut rec) => {
                while let Some(message) = rec.recv().await {
                    writeln!(lock, "{}", message).unwrap();
                    stdout().flush().unwrap();
                }
                assert_eq!(1, 1);
            }
            Err(e) => {
                println!("{:?}", e);
                assert_eq!(1, 0);
            }
        }
    }

    #[tokio::test]
    async fn test_chat() {
        // Test chat
        let id = "d5e80a2f-de3e-4b6f-951b-0250".to_string();
        let content = r"写一篇300字的日记".to_string();

        let mut lock = stdout().lock();

        match chat(id, content).await {
            Ok(mut rec) => {
                while let Some(message) = rec.recv().await {
                    writeln!(lock, "{}", message).unwrap();
                    stdout().flush().unwrap();
                }
                assert_eq!(1, 1);
            }
            Err(e) => {
                println!("{:?}", e);
                assert_eq!(1, 0);
            }
        }
    }
}
