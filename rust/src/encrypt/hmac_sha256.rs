use ring::{digest, hmac};

fn hmac_sha256(key: &[u8], data: &[u8]) -> hmac::Signature {
    let key = hmac::Key::new(hmac::HMAC_SHA256, key);
    let signature = hmac::sign(&key, data);

    signature
}

// 写一个关于 hmac_sha256 的单元测代码
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_hmac_sha256() {
        let key = b"super-secret-key";
        let data = b"Hello World!";
        let signature = hmac_sha256(key, data);
        println!("{:?}", signature.as_ref());
        assert_eq!(signature.as_ref(), &[
            0x4b, 0x39, 0x3a, 0xbc, 0xed, 0x1c, 0x49, 0x7f, 0x80, 0x48,
            0x86, 0x0b, 0xa1, 0xed, 0xe4, 0x6a, 0x23, 0xf1, 0xff, 0x52,
            0x09, 0xb1, 0x8e, 0x9c, 0x42, 0x8b, 0xdd, 0xfb, 0xb6, 0x90,
            0xaa, 0xd8]
        );
    }
}

