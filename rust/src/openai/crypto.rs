use std::fs;

use sha2::Sha256;
use digest::Digest;
use anyhow::Result;
use rsa::{pkcs8::{DecodePublicKey}, Oaep, RsaPublicKey};

use base64::{Engine as _, engine::general_purpose};

/// Calculates the SHA256 digest of the given data.
///
/// # Arguments
///
/// * `data` - The data to calculate the digest for.
///
/// # Returns
///
/// Returns the SHA256 digest as a hexadecimal string.
pub fn digest(data: &str) -> String {
    // create a Sha256 object
    let mut hasher = Sha256::new();

    // write input message
    hasher.update(data);

    // read hash digest and consume hasher
    let result = hasher.finalize();

    format!("{:x}", result)
}

/// Encrypts the given data using RSA encryption algorithm.
///
/// # Arguments
///
/// * `public_key_pem` - The public key in PEM format.
/// * `data` - The data to be encrypted.
///
/// # Returns
///
/// Returns the encrypted data as a base64-encoded string.
pub fn rsa_encrypt(public_key_pem: &str, data: &str) -> String {
    let a = data.as_bytes();
    let mut rng = rand::thread_rng();

    let pub_key: RsaPublicKey = RsaPublicKey::from_public_key_pem(public_key_pem).unwrap();

    // Encrypt the data using RSA-OAEP with SHA-256
    let padding = Oaep::new::<Sha256>();
    let enc_data = pub_key.encrypt(&mut rng, padding, a).expect("failed to encrypt");
    // Encode the encrypted data as a base64 string
    general_purpose::STANDARD.encode(enc_data.as_slice())
}

/// Encrypts the given data and returns the encrypted data and its digest.
///
/// # Arguments
///
/// * `data` - The data to be encrypted.
///
/// # Returns
///
/// Returns a tuple containing the digest and the encrypted data.
pub fn encrypt_id(data: &str) -> Result<(String,String)>{
    // Read the public key from the file
    let public_key_pem = fs::read_to_string(r"src\key\public_key.pem")?;
    // Encrypt the data using the public key
    let enc_data = rsa_encrypt(&public_key_pem, data);

    Ok((digest(data),enc_data))
}

// 测试用例
#[cfg(test)]
mod tests {
    use std::fs;

    use super::*;

    #[test]
    fn test_encrypt() {
        let data = "d5e80a2f-de3e-4b6f-951b-0250e455f329";
        let result = digest(data);
        assert_eq!(result, "60579a9e7fe6916e5e896d02a19b8456b4487c9f7f2bba81135d74a8a11f647e");
    }

    #[test]
    fn test_rsa_encrypt() {
        let public_key_pem = fs::read_to_string(r"src\key\public_key.pem").unwrap();
        let data = "d5e80a2f-de3e-4b6f-951b-0250e455f329";
        let enc_data = rsa_encrypt(&public_key_pem, data);
        println!("enc_data: {}", enc_data);
        // assert_eq!(enc_data, r"o2+3P7JrmITvv8MJrHtyyyRsf8Pyrd8NF0ytxmatF3xvGOj3GEZJkPAHA++sPSBSNAS6VCd6LbmN7SnFvu102fpPRkKT7Cokt3/hB4IkBx5f6Ok1qBJmzNC/WPTmxewpB65Nb4vTXtAF875ATw3f/T4rwKV0Kf/ljgqTfi0UYz9rg4yLfs/Y373Zji3z3lmccXrJSjk2HIgPbrgukQJ8I3oq/e1nXxtaxa/gVMk++/SKX3PtWhpXkV4Cu5/btH2MDJQCPgbfS/QPRLnQGarVZTP6LdWUd9kfjI1N9VZEya5IrW/vlbIKSCfXB/CKdQN3LJnCIaEgm9zoVQSBjTvDsA==");
        assert_eq!(1,1)
    }

    #[test]
    fn test_encrypt_id() {
        let data = "d5e80a2f-de3e-4b6f-951b-0250e455f329";
        let (digest, enc_data) = encrypt_id(data).unwrap();
        println!("digest: {}", digest);
        println!("enc_data: {}", enc_data);
        assert_eq!(digest, "60579a9e7fe6916e5e896d02a19b8456b4487c9f7f2bba81135d74a8a11f647e");
        // assert_eq!(enc_data, r"o2+3P7JrmITvv8MJrHtyyyRsf8Pyrd8NF0ytxmatF3xvGOj3GEZJkPAHA++sPSBSNAS6VCd6LbmN7SnFvu102fpPRkKT7Cokt3/hB4IkBx5f6Ok1qBJmzNC/WPTmxewpB65Nb4vTXtAF875ATw3f/T4rwKV0Kf/ljgqTfi0UYz9rg4yLfs/Y373Zji3z3lmccXrJSjk2HIgPbrgukQJ8I3oq/e1nXxtaxa/gVMk++/SKX3PtWhpXkV4Cu5/btH2MDJQCPgbfS/QPRLnQGarVZTP6LdWUd9kfjI1N9VZEya5IrW/vlbIKSCfXB/CKdQN3LJnCIaEgm9zoVQSBjTvDsA==");
    }
}