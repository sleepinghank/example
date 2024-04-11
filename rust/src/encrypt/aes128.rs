use aes::cipher::{block_padding::NoPadding, BlockDecryptMut, BlockEncryptMut, KeyIvInit};
use anyhow::{anyhow, Result};

type Aes128CbcDec = cbc::Decryptor<aes::Aes128>;
type Aes128CbcEnc = cbc::Encryptor<aes::Aes128>;

// key
static key: [u8; 16] = [
    0xd8, 0xb5, 0x98, 0x48, 0xc7, 0x67, 0x0c, 0x94, 0xb2, 0x9b, 0x54, 0xd2, 0x37, 0x9e, 0x2e, 0x7a
];
// 偏移 iv
static iv: [u8; 16] = [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f];

/// AES128 CBC 解密
fn decrypt(encrypted_data: &mut Vec<u8>) -> Result<Vec<u8>> {
    let cipher = Aes128CbcDec::new(&key.into(), &iv.into());
    let decrypt_data = cipher.decrypt_padded_mut::<NoPadding>(encrypted_data).map_err(|e| anyhow!(e))?; // 解密失败
    Ok(decrypt_data.to_vec())
}

/// AES128 CBC 加密
fn encrypt(decrypted_data: &mut Vec<u8>) -> Result<Vec<u8>> {
    let cipher = Aes128CbcEnc::new(&key.into(), &iv.into());
    let encrypt_data = cipher.encrypt_padded_mut::<NoPadding>(decrypted_data, 16).map_err(|e| anyhow!(e))?;
    Ok(encrypt_data.to_vec())
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_encrypt() {
        let mut data = vec![
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
        ];
        let result = encrypt(&mut data).unwrap();

        let hex_string = result.iter().map(|&byte| format!("0x{:02x}", byte)).collect::<Vec<String>>().join(",");
        println!("Hex Output: {:?}", hex_string);

        assert_eq!(result, [
            0x80, 0xD6, 0x21, 0x70, 0x7F, 0x64, 0xF5, 0x65, 0x84, 0x0, 0xA, 0x68, 0xAD, 0xCB, 0xB7, 0xE4
        ]);
    }

    #[test]
    fn test_decrypt() {
        let mut data = vec![
            0x80, 0xD6, 0x21, 0x70, 0x7F, 0x64, 0xF5, 0x65, 0x84, 0x0, 0xA, 0x68, 0xAD, 0xCB, 0xB7, 0xE4
        ];
        let result = decrypt(&mut data).unwrap();

        let hex_string = result.iter().map(|&byte| format!("0x{:02x}", byte)).collect::<Vec<String>>().join(",");
        println!("Hex Output: {:?}", hex_string);

        assert_eq!(result, [
            0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
        ]);
    }
}
