use aes_gcm::aead::{Aead, KeyInit};
use aes_gcm::aes::Aes256;
use aes_gcm::{Aes256Gcm, Key, Nonce};
use rand::{rngs::OsRng, RngCore};

pub const FACE_SNAPSHOT_KEY_LEN: usize = 32;
const NONCE_LEN: usize = 12;
const SNAPSHOT_CONTEXT: &[u8] = b"face-snapshot";
const METADATA_CONTEXT: &[u8] = b"face-snapshot-meta";

fn derive_key_from_hex(hex_str: &str) -> Result<Key<Aes256>, String> {
    let bytes = hex::decode(hex_str)
        .map_err(|err| format!("invalid faceSnapshotKey hex: {err}"))?;
    if bytes.len() != FACE_SNAPSHOT_KEY_LEN {
        return Err(format!(
            "faceSnapshotKey must decode to {} bytes, got {}",
            FACE_SNAPSHOT_KEY_LEN,
            bytes.len()
        ));
    }
    Ok(Key::<Aes256>::from_slice(&bytes).clone())
}

fn random_nonce() -> [u8; NONCE_LEN] {
    let mut nonce = [0u8; NONCE_LEN];
    OsRng.fill_bytes(&mut nonce);
    nonce
}

fn build_cipher(key_hex: &str) -> Result<Aes256Gcm, String> {
    let key = derive_key_from_hex(key_hex)?;
    Ok(Aes256Gcm::new(&key))
}

fn encrypt_blob(cipher: &Aes256Gcm, context: &[u8], plaintext: &[u8]) -> Result<Vec<u8>, String> {
    let nonce_bytes = random_nonce();
    let nonce = Nonce::from_slice(&nonce_bytes);
    let mut aad = Vec::with_capacity(context.len());
    aad.extend_from_slice(context);

    let mut ciphertext = cipher
        .encrypt(nonce, aes_gcm::aead::Payload { msg: plaintext, aad: &aad })
        .map_err(|err| format!("encryption failed: {err}"))?;
    let mut result = Vec::with_capacity(NONCE_LEN + ciphertext.len());
    result.extend_from_slice(&nonce_bytes);
    result.append(&mut ciphertext);
    Ok(result)
}

fn decrypt_blob(cipher: &Aes256Gcm, context: &[u8], payload: &[u8]) -> Result<Vec<u8>, String> {
    if payload.len() <= NONCE_LEN {
        return Err("ciphertext truncated".to_string());
    }
    let (nonce_part, data_part) = payload.split_at(NONCE_LEN);
    let nonce = Nonce::from_slice(nonce_part);
    let mut aad = Vec::with_capacity(context.len());
    aad.extend_from_slice(context);

    cipher
        .decrypt(nonce, aes_gcm::aead::Payload { msg: data_part, aad: &aad })
        .map_err(|err| format!("decryption failed: {err}"))
}

pub fn encrypt_snapshot_bytes(key_hex: &str, data: &[u8]) -> Result<Vec<u8>, String> {
    let cipher = build_cipher(key_hex)?;
    encrypt_blob(&cipher, SNAPSHOT_CONTEXT, data)
}

pub fn encrypt_snapshot_metadata(key_hex: &str, data: &[u8]) -> Result<Vec<u8>, String> {
    let cipher = build_cipher(key_hex)?;
    encrypt_blob(&cipher, METADATA_CONTEXT, data)
}

#[allow(dead_code)]
pub fn decrypt_snapshot_bytes(key_hex: &str, payload: &[u8]) -> Result<Vec<u8>, String> {
    let cipher = build_cipher(key_hex)?;
    decrypt_blob(&cipher, SNAPSHOT_CONTEXT, payload)
}

#[allow(dead_code)]
pub fn decrypt_snapshot_metadata(key_hex: &str, payload: &[u8]) -> Result<Vec<u8>, String> {
    let cipher = build_cipher(key_hex)?;
    decrypt_blob(&cipher, METADATA_CONTEXT, payload)
}
