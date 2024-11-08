
use async_openai::types::{ChatCompletionRequestUserMessageArgs, CreateChatCompletionRequestArgs};
use async_openai::Client;
use async_openai::config::Config;
use anyhow::{Result,anyhow};
use reqwest::header::{HeaderMap, AUTHORIZATION};

use std::io::{stdout, Write};
use futures::StreamExt;
use secrecy::{ExposeSecret, Secret};

#[derive(Clone)]
struct ApiConfig {
    api_base: String,
    api_key: Secret<String>,
}

impl ApiConfig {
    pub fn new<S: Into<String>>(api_base: S, api_key: S) -> Self {
        Self { api_base: api_base.into(), api_key:  Secret::from(api_key.into()) }
    }
}

impl Config for ApiConfig {
    fn headers(&self) -> HeaderMap {
        let mut headers = HeaderMap::new();

        headers.insert(
            AUTHORIZATION,
            format!("Bearer {}", self.api_key.expose_secret())
                .as_str()
                .parse()
                .unwrap(),
        );

        headers
    }

    fn url(&self, path: &str) -> String {
        format!("{}{}", self.api_base(), path)
    }

    fn api_base(&self) -> &str {
        &self.api_base
    }

    fn api_key(&self) -> &Secret<String> {
        &self.api_key
    }

    fn query(&self) -> Vec<(&str, &str)> {
        vec![]
    }
}

/// 获取本地 AI 服务的健康状态 请求http://127.0.0.1:8080/health
/// 分三种情况：
/// 1. 服务正常，返回 200 OK
/// 2. 服务未准备完成，返回 500 Internal Server Error
/// 3. 服务不可用，返回 503 Service Unavailable
pub async fn local_ai_health() -> Result<bool>{
    let client = reqwest::Client::new();
    let result = client
        .get("http://127.0.0.1:8080/health")
        .send()
        .await;
    
    match result {
        Ok(response) => {
            // 检查响应状态
            match response.status().as_u16() {
                200 => Ok(true),
                _ => Ok(false),
            }
        }
        Err(_) => Err(anyhow!("Local ai service is unavailable"))
    }
}

pub async fn test(req:String) -> Result<()>{
    let config = ApiConfig::new(r"http://127.0.0.1:8080", "");
    let client = Client::with_config(config);

    let request = CreateChatCompletionRequestArgs::default()
        // .model("inateckai")
        .max_tokens(512u16)
        .messages([ChatCompletionRequestUserMessageArgs::default()
            .content(req)
            .build()?
            .into()])
        .build()?;

    let mut stream = client.chat().create_stream(request).await?;

    let mut count = 0;
    let mut lock = stdout().lock();
    while let Some(result) = stream.next().await {
        match result {
            Ok(response) => {
                count += 1;
                response.choices.iter().for_each(|chat_choice| {
                    if let Some(ref content) = chat_choice.delta.content {
                        write!(lock, "{}", content).unwrap();
                    }
                });
            }
            Err(err) => {
                writeln!(lock, "error: {err}").unwrap();
                break;
            }
        }
        stdout().flush()?;
    }

    Ok(())
}