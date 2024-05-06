pub mod crc16;
pub mod aes128;

use aes::cipher::{KeyIvInit};


type Aes128CbcDec = cbc::Decryptor<aes::Aes128>;
type Aes128CbcEnc = cbc::Encryptor<aes::Aes128>;

const MAX_AES_KEY_SIZE: usize = 8;

const ORIGINAL_KEY: [[u8; 16]; MAX_AES_KEY_SIZE] = [
    [ 0xd8, 0x4f, 0x97, 0x1a, 0xe3, 0x5b, 0x2e, 0x73, 0xc6, 0x8d, 0xf2, 0x6a, 0xbc, 0x91, 0x3f, 0x59 ],
    [ 0xa7, 0x3e, 0x9c, 0x51, 0x6d, 0x8f, 0x2b, 0xe7, 0x5a, 0xc3, 0xf9, 0x64, 0x1b, 0xd0, 0x78, 0xaf ],
    [ 0x38, 0x90, 0xde, 0x14, 0x6c, 0x2f, 0x8d, 0xb1, 0x7a, 0x45, 0xef, 0x09, 0x53, 0xbc, 0x1d, 0x86 ],
    [ 0xf5, 0x23, 0x7c, 0x9e, 0x41, 0xd7, 0x5a, 0x82, 0x06, 0xba, 0x3f, 0x98, 0xe1, 0x65, 0xc4, 0x17 ],
    [ 0x69, 0xb2, 0x87, 0xf4, 0x32, 0xae, 0x5c, 0xd9, 0x01, 0x76, 0xed, 0x48, 0x9f, 0x20, 0xcb, 0x54 ],
    [ 0x71, 0x28, 0xbe, 0x03, 0x59, 0xe2, 0x95, 0xf8, 0x47, 0x0d, 0xa6, 0x31, 0xcd, 0x6f, 0x84, 0xb2 ],
    [ 0xea, 0x67, 0x25, 0xd1, 0x08, 0xf6, 0x72, 0xbc, 0x93, 0x4e, 0xa5, 0x30, 0x5b, 0x19, 0xc4, 0x7d ],
    [ 0x12, 0x96, 0x43, 0xcf, 0x5e, 0x18, 0x79, 0xdb, 0xa3, 0x24, 0x6d, 0xf8, 0xb7, 0x50, 0xe1, 0xac ]
];

const ORIGINAL_IV: [[u8; 16]; MAX_AES_KEY_SIZE] =[
    [ 0x5f, 0xa2, 0x38, 0xd6, 0x90, 0x17, 0xbe, 0x7d, 0xc4, 0x52, 0xe9, 0x03, 0x86, 0xf1, 0x6a, 0xbd ],
    [ 0x41, 0xda, 0x87, 0x29, 0x50, 0xec, 0x13, 0xbf, 0x78, 0x25, 0x95, 0x61, 0xd3, 0x0f, 0xac, 0x7e ],
    [ 0x20, 0x9e, 0x73, 0x85, 0x4a, 0xc6, 0x0b, 0xf9, 0x5d, 0x27, 0xd8, 0x64, 0xb1, 0x3f, 0xae, 0x70 ],
    [ 0xcb, 0x5f, 0x18, 0x9e, 0x72, 0xd5, 0xa3, 0x07, 0xf1, 0x4c, 0xe9, 0x63, 0xb0, 0x28, 0x84, 0xad ],
    [ 0x9a, 0x2d, 0xf7, 0x45, 0x88, 0x1c, 0x67, 0xbd, 0x30, 0xe6, 0x59, 0xaf, 0x14, 0xd3, 0x82, 0x4b ],
    [ 0x6e, 0xd5, 0x81, 0x2f, 0x47, 0xb9, 0x16, 0xfa, 0x9c, 0x53, 0xe0, 0x7a, 0x34, 0xc8, 0xae, 0x12 ],
    [ 0x3c, 0x97, 0x68, 0xae, 0x1d, 0x58, 0xfc, 0x74, 0xe6, 0x32, 0x8b, 0xd0, 0x49, 0xa7, 0x15, 0xf2 ],
    [ 0x57, 0xf4, 0x89, 0x23, 0xe7, 0x4b, 0x16, 0x9d, 0x60, 0xca, 0x05, 0xb8, 0x3f, 0xad, 0x72, 0x10 ]
];


struct CryptoDevice {
    aes_keys: Vec<[u8; 16]>,
    aes_ivs: Vec<[u8; 16]>,
    mac_address: [u8; 6],
    product_code: u32,
    chip_code: u32,
}

impl CryptoDevice {
    fn new(mac_address: [u8; 6], product_code: u32, chip_code: u32) -> CryptoDevice {
        let mut aes_keys = Vec::new();
        let mut aes_ivs = Vec::new();
        for z in 0..MAX_AES_KEY_SIZE {
            let mut key = [0u8; 16];
            for i in 0..16 {
                match i {
                    0 | 2 => key[i] = mac_address[0] ^ ORIGINAL_KEY[z][i],
                    1 | 3 => key[i] = mac_address[1] ^ ORIGINAL_KEY[z][i],
                    4 | 6 => key[i] = mac_address[2] ^ ORIGINAL_KEY[z][i],
                    5 | 7 => key[i] = mac_address[3] ^ ORIGINAL_KEY[z][i],
                    8 | 10 => key[i] = mac_address[4] ^ ORIGINAL_KEY[z][i],
                    9 | 11 => key[i] = mac_address[5] ^ ORIGINAL_KEY[z][i],
                    12 => key[i] = (mac_address[0] + mac_address[2]) ^ ORIGINAL_KEY[z][i],
                    13 => key[i] = (mac_address[1] - mac_address[3]) ^ ORIGINAL_KEY[z][i],
                    14 => key[i] = (mac_address[2] + mac_address[4]) ^ ORIGINAL_KEY[z][i],
                    15 => key[i] = (mac_address[3] - mac_address[5]) ^ ORIGINAL_KEY[z][i],
                    _ => continue,
                }
            }
            aes_keys.push(key);

            let mut iv = [0u8; 16];
            for i in 0..16 {
                match i {
                    0 | 2 => iv[i] =((product_code >> 24) & 0xFF) as u8,
                    1 | 3 => iv[i] = ((product_code >> 16) & 0xFF) as u8,
                    4 | 6 => iv[i] = ((product_code >> 8) & 0xFF) as u8,
                    5 | 7 => iv[i] = (product_code  & 0xFF) as u8,
                    8 | 10 => iv[i] = ((chip_code >> 24) & 0xFF) as u8,
                    9 | 11 => iv[i] =((chip_code >> 16) & 0xFF) as u8,
                    12 | 14 => iv[i] = ((chip_code >> 8) & 0xFF) as u8,
                    13 | 15 => iv[i] = (chip_code  & 0xFF) as u8,
                    _ => continue,
                }
    
                iv[i] ^= ORIGINAL_IV[z][i];
            }
            aes_ivs.push(iv);
        }
        CryptoDevice {
            aes_keys,
            aes_ivs,
            mac_address,
            product_code,
            chip_code,
        }
    }

   //  fn encrypt(&self, plain_data: &[u8], key_index: usize) -> Result<Vec<u8>> {
   //      let cipher = Aes128CbcEnc::new(&self.aes_keys[key_index], &self.aes_ivs[key_index]);
   //      let encrypt_data = cipher.encrypt_padded_mut::<NoPadding>(plain_data, 16).map_err(|e| anyhow!(e))?;
   //      Ok(encrypt_data.to_vec())
   //  }
   //
   //  /// 数据包格式
   //  /// | 简称 | Command Code | Error Code | Encrypt Data Length | Encrypt Data CRC16 |  Key Index  | Random Number | Encrypt Data |
   // /// | :--: | :----------: | :--------: | :-----------------: | :----------------: | :---------: | :-----------: | :----------: |
   // /// | 长度 |    1 Byte    |   1 Byte   |       1 Byte        |       2 Byte       |   1 Byte    |    4 Byte     |   Len Byte   |
   // /// | 描述 | 命令码（+1） |   错误码   |   加密包数据长度    | 加密数据CRC16校验  | 秘钥（0~7） |    随机数     |   加密数据   |
   //  fn decrypt(&self, cipher_data: &[u8], key_index: usize) -> Result<Vec<u8>> {
   //      let cipher = Aes128CbcDec::new(&self.aes_keys[key_index], &self.aes_ivs[key_index]);
   //      let decrypt_data = cipher.decrypt_padded_mut::<NoPadding>(cipher_data).map_err(|e| anyhow!(e))?; // 解密失败
   //      Ok(decrypt_data.to_vec())
   //  }
}