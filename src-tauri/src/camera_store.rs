#[tauri::command]
pub fn remove_camera(ip: String) -> Result<(), String> {
    let path = get_camera_file();
    let data = std::fs::read_to_string(&path).map_err(|e| e.to_string())?;
    let mut cameras: Vec<Camera> = serde_json::from_str(&data).map_err(|e| e.to_string())?;
    cameras.retain(|c| c.ip != ip);
    let json = serde_json::to_string_pretty(&cameras).map_err(|e| e.to_string())?;
    std::fs::write(path, json).map_err(|e| e.to_string())
}
use base64::{engine::general_purpose, Engine as _};
use serde::{Deserialize, Serialize};
// use std::fs; // Неиспользуемый импорт
use aes_gcm::aead::Aead;
use aes_gcm::aes::Aes256;
use aes_gcm::KeyInit;
use aes_gcm::{Aes256Gcm, Key, Nonce};
use dirs_next::data_dir;
use std::path::PathBuf;

const AES_KEY: &[u8; 32] = b"0123456789abcdef0123456789abcdef"; // заменить на свой ключ

#[derive(Serialize, Deserialize, Clone)]
pub struct Camera {
    pub id: u32,
    pub name: String,
    pub ip: String,
    pub protocol: String,
    pub port: u16,
    pub user: String,
    #[serde(default)]
    pub pass: String, // plain password (will be encrypted when saving)
    #[serde(default)]
    pub pass_enc: String, // encrypted password
    #[serde(default)]
    pub path_hd: String,
    #[serde(default)]
    pub path_sd: String,
    #[serde(default)]
    pub status: String,
    #[serde(rename = "onvifPort")]
    pub onvif_port: Option<u16>,
    #[serde(rename = "groupId")]
    pub group_id: Option<u32>,
    // Дополнительные поля для совместимости
    pub main_stream: Option<String>,
    pub sub_stream: Option<String>,
    pub brand: Option<String>,
    #[serde(rename = "isConnected")]
    pub is_connected: Option<bool>,
    #[serde(rename = "streamUrl")]
    pub stream_url: Option<String>,
}

#[derive(Serialize, Deserialize, Clone)]
pub struct CameraGroup {
    pub id: u32,
    pub name: String,
    pub camera_ids: Vec<u32>,
    pub created_at: String,
}

fn get_camera_file() -> PathBuf {
    let mut dir = data_dir().unwrap_or_else(|| std::env::current_dir().unwrap());
    dir = dir.join("OpenIPC-VMS");
    std::fs::create_dir_all(&dir).ok();
    dir.join("cameras.json")
}

#[tauri::command]
pub fn save_cameras(cameras: Vec<Camera>) -> Result<(), String> {
    let path = get_camera_file();
    println!("Saving {} cameras to: {:?}", cameras.len(), path);

    let json = serde_json::to_string_pretty(&cameras).map_err(|e| {
        println!("Failed to serialize cameras: {}", e);
        e.to_string()
    })?;

    println!("Camera JSON to save: {}", json);

    std::fs::write(&path, json).map_err(|e| {
        println!("Failed to write camera file: {}", e);
        e.to_string()
    })?;

    println!("Cameras saved successfully");
    Ok(())
}

#[tauri::command]
pub fn load_cameras() -> Result<Vec<Camera>, String> {
    let path = get_camera_file();
    if !path.exists() {
        println!("Camera file does not exist: {:?}", path);
        return Ok(vec![]);
    }

    match std::fs::read_to_string(&path) {
        Ok(data) => {
            println!("Loading cameras from: {:?}", path);
            println!("Camera file content: {}", data);

            match serde_json::from_str::<Vec<Camera>>(&data) {
                Ok(cameras) => {
                    println!("Successfully loaded {} cameras", cameras.len());
                    Ok(cameras)
                }
                Err(e) => {
                    println!("Failed to parse cameras JSON: {}", e);
                    Err(format!("Failed to parse cameras: {}", e))
                }
            }
        }
        Err(e) => {
            println!("Failed to read camera file: {}", e);
            Err(e.to_string())
        }
    }
}

fn get_groups_file() -> PathBuf {
    let mut dir = data_dir().unwrap_or_else(|| std::env::current_dir().unwrap());
    dir = dir.join("OpenIPC-VMS");
    std::fs::create_dir_all(&dir).ok();
    dir.join("groups.json")
}

#[tauri::command]
pub fn save_groups(groups: Vec<CameraGroup>) -> Result<(), String> {
    let json = serde_json::to_string_pretty(&groups).map_err(|e| e.to_string())?;
    let path = get_groups_file();
    std::fs::write(path, json).map_err(|e| e.to_string())
}

#[tauri::command]
pub fn load_groups() -> Result<Vec<CameraGroup>, String> {
    let path = get_groups_file();
    if !path.exists() {
        return Ok(vec![]);
    }
    let data = std::fs::read_to_string(path).map_err(|e| e.to_string())?;
    let groups: Vec<CameraGroup> = serde_json::from_str(&data).map_err(|e| e.to_string())?;
    Ok(groups)
}

#[tauri::command]
pub fn encrypt_password(password: String) -> Result<String, String> {
    let key = Key::<Aes256>::from_slice(AES_KEY);
    let cipher = Aes256Gcm::new(key);
    let nonce = Nonce::from_slice(b"unique_nonce"); // 12 bytes
    let ciphertext = cipher
        .encrypt(nonce, password.as_bytes())
        .map_err(|e| e.to_string())?;
    Ok(general_purpose::STANDARD.encode(ciphertext))
}

#[tauri::command]
pub fn decrypt_password(enc: &str) -> Result<String, String> {
    let key = Key::<Aes256>::from_slice(AES_KEY);
    let cipher = Aes256Gcm::new(key);
    let nonce = Nonce::from_slice(b"unique_nonce");
    let ciphertext = general_purpose::STANDARD
        .decode(enc)
        .map_err(|e| e.to_string())?;
    let plaintext = cipher
        .decrypt(nonce, ciphertext.as_ref())
        .map_err(|e| e.to_string())?;
    Ok(String::from_utf8(plaintext).map_err(|e| e.to_string())?)
}
