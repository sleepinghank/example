
use reqwest::Response;
use serde::{Deserialize, Serialize};
use futures_util::StreamExt;
use bytes::BytesMut;
use tokio::sync::mpsc::UnboundedReceiver;
use reqwest_eventsource::{Event, EventSource, RequestBuilderExt};

use super::crypto::encrypt_id;

/// 设备注册 请求
#[derive(Serialize, Deserialize, Debug,Clone,Default)]
pub struct DeviceRegisterRequest {
    // 设备 UUID
    id: String,
    // 产品基本信息
    product_info: ProductInfo,
    // 设备固件版本
    firmware_version: u32,
    // 桌面软件版本
    software_version: u32,
    // 加密使用，暂定
    encrypt: String,
}

impl DeviceRegisterRequest {
    pub fn new(id:String,device_name: String,product_model_code: u32,device_type: u8,firmware_version: u32,software_version: u32) -> Self{
        Self {
            id,
            product_info: ProductInfo{
                device_name,
                product_model_code,
                device_type,
            },
            firmware_version,
            software_version,
            encrypt: "encrypt".to_string(),
        }
    }
}

#[derive(Serialize, Deserialize, Debug,Clone,Default)]
pub struct ProductInfo {
    // 产品名称
    device_name: String,
    // 产品型号
    product_model_code: u32,
    // 产品类型 1:Keyboard 2:Mouse 3:Touchpad 4:MulKeyboardTouchpad 100:Other
    device_type: u8,
}

/// 翻译请求
#[derive(Serialize, Deserialize, Debug,Clone,Default)]
pub struct TranslationRequest {
    // 设备 UUID
    id: String,
    // UUID,会话id
    session_id: String,
    // 翻译目标语言
    target_lang: String,
    // 翻译原文本
    source_text: String,
    // 涉及加密内容，讨论方案后决定
    token: String,
}

/// AI 对话请求
#[derive(Serialize, Deserialize, Debug,Clone,Default)]
pub struct DialogueRequest {
    // 设备 UUID
    id: String,
    // 提问内容
    content: String,
}
/// 设备注册响应
#[derive(Serialize, Deserialize, Debug,Clone,Default)]
pub struct DeviceRegisterResponse {
    // 数据
    data: Option<serde_json::Value>,
    // 消息
    message: String,
    // 状态
    status: serde_json::Value,
}


const API_BASE: &str = r"http://176.58.108.46:4567/";

/// 设备请求注册接口，使用reqwest发起post请求，请求地址为  API_BASE + device_register
pub async fn device_register(id:&str,mut request:DeviceRegisterRequest) -> anyhow::Result<bool> {
    let (digest,base) = encrypt_id(id)?;
    request.id = base;
    request.encrypt = digest;
    let client = reqwest::Client::new();
    let response:DeviceRegisterResponse = client.post(format!("{}device_register", API_BASE))
        .json(&request)
        .send()
        .await?.json()
        .await?;
    println!("{:?}",response);
    Ok(true)
}

/// 设备请求注册接口，使用reqwest发起post请求，请求地址为  API_BASE + translate
pub async fn translate(id: String, session_id: String, target_lang: String, source_text: String) -> anyhow::Result<UnboundedReceiver<String>> {
    let request = TranslationRequest {
        id,
        session_id,
        target_lang,
        source_text,
        token:"token".to_string(),
    };

    let client = reqwest::Client::new();
    let response = client.post(format!("{}translate", API_BASE))
        .json(&request)
        .send().await?;
    let receiver = buffer_respose(response);
    Ok(receiver)


    // let mut event_source: EventSource = client.post(format!("{}translate", API_BASE))
    //     .json(&request)
    // .eventsource()?;

    // stream_response(event_source).await
}

fn buffer_respose(response: Response) -> UnboundedReceiver<String> {
    let (tx, rx) = tokio::sync::mpsc::unbounded_channel();
    // 在一个新的任务中处理流
    tokio::spawn(async move {
        // // 创建一个新的行流
        let mut stream = response.bytes_stream();

        let mut buffer = BytesMut::with_capacity(20);

        // 从流中读取数据
        while let Some(result) = stream.next().await {
            match result {
                Ok(bytes) => {
                    let size = bytes.len();
                    if size == 0 {
                        // 流已经关闭
                        break;
                    }
                    // let data: String = String::from_utf8_lossy(&bytes).to_string();
                    // if let Err(e) = tx.send(data){
                    //     println!("sender send error:{:?}",e);
                    //     break;
                    // };

                    buffer.extend_from_slice(&bytes);
                    println!("buffer size:{}",buffer.len());
                    // 当缓冲区的大小达到 2000 时，将缓冲区的内容发送到 sender
                    if buffer.len() >= 16 {
                        let data: String = String::from_utf8_lossy(&buffer).to_string();
                        if let Err(e) = tx.send(data){
                            println!("sender send error:{:?}",e);
                            break;
                        };
                        buffer.clear();
                    }
                }
                Err(err) => {
                    println!("receiver translate error:{:?}",err);
                    break;
                }
            }
        }
        // 如果流关闭时缓冲区中还有数据，将剩余的数据发送到 sender
        if !buffer.is_empty() {
            if let Err(e) = tx.send(String::from_utf8_lossy(&buffer.to_vec()).to_string()){
                println!("sender send error:{:?}",e);
                return;
            };
        };
        if let Err(e) = tx.send("[DONE]".to_string()){
            println!("sender send error:{:?}",e);
        };
    });
    rx
}

// pub async fn stream_response(mut event_source: EventSource) -> anyhow::Result<UnboundedReceiver<String>> {
//     let (tx, rx) = tokio::sync::mpsc::unbounded_channel();

//     tokio::spawn(async move {
//         while let Some(ev) = event_source.next().await {
//             match ev {
//                 Err(e) => {
//                     // if let Err(_e) = tx.send(Err(OpenAIError::StreamError(e.to_string()))) {
//                     //     // rx dropped
//                     //     break;
//                     // }
//                     println!("{:?}",e);
//                     break;
//                 }
//                 Ok(event) => match event {
//                     Event::Message(message) => {
//                         if message.data == "[DONE]" {
//                             break;
//                         }
//                         let a = message.data;
//                         // let response = match serde_json::from_str::<O>(&) {
//                         //     Err(e) => Err(map_deserialization_error(e, message.data.as_bytes())),
//                         //     Ok(output) => Ok(output),
//                         // };
//                         println!("{:?}",a);
//                         if let Err(_e) = tx.send(a) {
//                             // rx dropped
//                             break;
//                         }
//                     }
//                     Event::Open => continue,
//                 },
//             }
//         }

//         event_source.close();
//     });

//     Ok(rx)
// }


/// 请求单次聊天，使用reqwest发起post请求，请求参数为DialogueRequest，请求地址为  API_BASE + chat，使用bytes_stream 处理流式响应
pub async fn chat(id: String, content: String) -> anyhow::Result<UnboundedReceiver<String>> {
    let request = DialogueRequest {
        id,
        content,
    };

    let client = reqwest::Client::new();
    let response = client.post(format!("{}chat", API_BASE))
        .json(&request)
        .send().await?;

    
    let receiver = buffer_respose(response);
    Ok(receiver)
}

/// 编写两个方法的测试用例
#[cfg(test)]
mod tests {
    use super::*;
    use std::io::{stdout, Write};
    #[tokio::test]
    async fn test_device_register() {
        let id =
            "d5e80a2f-de3e-4b6f-951b-0250e455f329".to_string();
        let product_info = ProductInfo {
            device_name: "KB06103".to_string(),
            product_model_code: 1048657,
            device_type: 1,
        };
        let firmware_version = 265;
        let software_version = 0;
        let request = DeviceRegisterRequest {
            id:"".to_string(),
            product_info,
            firmware_version,
            software_version,
            encrypt:"encrypt".to_string(),
        };
        let result = device_register(id.as_str(),request).await;
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
        let id =
            "d5e80a2f-de3e-4b6f-951b-0250e455f329".to_string();
        let session_id = "d5e80a2f-de3e-4b6f-951b-0250e455f329".to_string();
        let target_lang = "English".to_string();
        let source_text = r"假如今天是我生命中的最后一天。 我要如何利用这最后、最宝贵的一天呢?首先，我要把一天的时间珍藏好，不让一分一秒的时间滴漏。
        我不为昨日的不幸叹息，过去的已够不幸，不要再陪上今日的运道。 时光会倒流吗?太阳会西升东落吗?我可以纠正昨天的错误吗?我能抚平昨日的创伤吗?
        我能比昨天年轻吗?一句出口的恶言，一记挥出的拳头，一切造成的痛，能收回吗? 不能!过去的永远过去了，我不再去想它。 假如今天是我生命中的最后一天。 
        我该怎么办?忘记昨天，也不要痴想明天。明天是一个未知数，为什么要把今天的精力浪费在未知的事上?想着明天的种种，今天的时光也白白流失了。
        祈盼今早的太阳再次升起，太阳已经落山。走在今天的路上，能做明天的事吗?我能把明天的金币放进今天的钱袋吗?明日瓜熟，今日能蒂落吗?
        明天的死亡能将今天的欢乐蒙上阴影吗?我能杞人忧天吗?明天和明天一样被我埋葬。我不再想它。 今天是我生命中的最后一天。 这是我仅有的一天，是现实的永恒。
        我像被赦免死刑的囚犯，用喜悦的泪水拥抱新生的太阳。我举起双手，感谢这无与伦比的一天。
        当我想到昨天和我一起迎接日出的朋友，今天已不复存在时，我为自己的幸存，感激上苍。
        我是无比幸运的人，今天的时光是额外的奖赏。许多强者都先我而去，为什么我得到这额外的一天?
        是不是因为他们已大功告成，而我尚在旅途跋涉?如果这样，这是不是成就我的一次机会，让我功德圆满?
        造物主的安排是否别具匠心? 今天是不是我超越他人的机会?今天是我生命中的最后一天。 
        生命只有一次，而人生也不过是时间的累积。我若让今天的时光白白流失，就等于毁掉人生最后一页。
        因此，我珍惜今天的一分一秒，因为他们将一去不复返。
        我无法把今天存入银行，明天再来取用。时间像风一样不可捕捉。
        每一分一秒，我要用双手捧住，用爱心抚摸，因为他们如此宝贵。垂死的人用毕生的钱财都无法换得一口生气。
        我无法计算时间的价值，它们是无价之宝! 今天是我生命中的最后一天。 我憎恨那些浪费时间的行为。
        我要摧毁拖延的习性。我要以真诚埋葬怀疑，用信心驱赶恐惧。我不听闲话，不游手好闲，不与不务正业的人来往。
        我终于醒悟到，若是懒惰，无异于从我所爱之人手中窃取食物和衣裳。我不是贼，我有爱心，今天是我最后的机会，我要证明我的爱心和伟大。 
        今天是我生命中的最后一天。 今日事今日毕。今天我要趁孩子还小的时侯，多加爱护，明天他们将离我而去，我也会离开。
        今天我要深 情地拥抱我的妻子，给她甜蜜的热吻，明天她会离去，我也是。今天我要帮助落难的朋友，明天他不再求援，我也听不到他的哀求。
        我要乐于奉献，因为明天我无法给予，也没有人来领受了。 今天是我生命中的最后一天。 如果这是我的末日，那么它就是不朽的纪念日。
        我把它当成最美好的日子。我要把每分每秒化为甘露，一口一口，细细品尝，满怀感激。我要每一分钟都有价值。我要加倍努力，直到精疲力竭。
        即使这样，我还要继续努力。今天的每一分钟都胜过昨天的每一小时，最后的也是最好的。 假如今天是我生命中的最后一天?? 如果不是的话，我要跪倒在上苍面前，深深致谢。".to_string();
        let mut lock = stdout().lock();
        match translate(id, session_id, target_lang, source_text).await {
            Ok(mut rec) => {
                while let Some(message) = rec.recv().await {
                    write!(lock, "{}\n", message).unwrap();
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
        let id =
            "d5e80a2f-de3e-4b6f-951b-0250e455".to_string();
        let content = r"写一篇300字的日记".to_string();
        let mut lock = stdout().lock();
        match chat(id, content).await {
            Ok(mut rec) => {
                while let Some(message) = rec.recv().await {
                    write!(lock, "{}\n", message).unwrap();
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
