
use async_openai::types::{ChatCompletionRequestMessage, ChatCompletionRequestUserMessageArgs, CreateChatCompletionRequest, CreateChatCompletionRequestArgs, CreateCompletionRequestArgs, Role};
use async_openai::Client;
use async_openai::config::{AzureConfig, Config, OPENAI_ORGANIZATION_HEADER, OpenAIConfig};
use anyhow::Result;
use reqwest::header::{HeaderMap, AUTHORIZATION};
use std::error::Error;
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



pub async fn test(req:String) -> Result<()>{
    let config = ApiConfig::new(r"http://rx4090.inateck.top:8080", "inateck.com");
    let client = Client::with_config(config);

    let request = CreateChatCompletionRequestArgs::default()
        .model("inateckai")
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
                // write!(lock, "{}: ", count).unwrap();
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