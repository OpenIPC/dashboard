use lazy_static::lazy_static;
use parking_lot::RwLock;
use serde::{Deserialize, Serialize};
use std::fs;
use std::path::PathBuf;
use std::sync::Arc;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AnprConfig {
    pub detection_confidence: f32,
    pub crop_expansion_factor: f32,
    pub crnn_confidence_threshold: f32,
    pub python_confidence_threshold: f32,
    pub enable_python_ocr: bool,
}

impl Default for AnprConfig {
    fn default() -> Self {
        Self {
            detection_confidence: 0.5,
            crop_expansion_factor: 1.2,
            crnn_confidence_threshold: 0.75,
            python_confidence_threshold: 0.90,
            enable_python_ocr: true,
        }
    }
}

impl AnprConfig {
    #[allow(dead_code)]
    pub fn load_from_file(path: &PathBuf) -> Result<Self, String> {
        if !path.exists() {
            // Create default config file if it doesn't exist
            let default_config = Self::default();
            default_config.save_to_file(path)?;
            return Ok(default_config);
        }

        let content =
            fs::read_to_string(path).map_err(|e| format!("Failed to read ANPR config: {}", e))?;

        let config: Self = serde_json::from_str(&content)
            .map_err(|e| format!("Failed to parse ANPR config: {}", e))?;

        Ok(config)
    }

    #[allow(dead_code)]
    pub fn save_to_file(&self, path: &PathBuf) -> Result<(), String> {
        if let Some(parent) = path.parent() {
            fs::create_dir_all(parent)
                .map_err(|e| format!("Failed to create config directory: {}", e))?;
        }

        let content = serde_json::to_string_pretty(self)
            .map_err(|e| format!("Failed to serialize ANPR config: {}", e))?;

        fs::write(path, content).map_err(|e| format!("Failed to write ANPR config: {}", e))?;

        Ok(())
    }
}

// Global config instance with thread-safe access
lazy_static! {
    pub static ref ANPR_CONFIG: Arc<RwLock<AnprConfig>> =
        Arc::new(RwLock::new(AnprConfig::default()));
}

#[tauri::command]
pub fn get_anpr_config() -> AnprConfig {
    ANPR_CONFIG.read().clone()
}

#[allow(dead_code)]
pub fn update_config(new_config: AnprConfig) {
    let mut config = ANPR_CONFIG.write();
    *config = new_config;
}

#[allow(dead_code)]
pub fn get_config_path() -> PathBuf {
    let mut path = dirs::config_dir().unwrap_or_else(|| PathBuf::from("."));
    path.push("openipc-vms");
    path.push("anpr_config.json");
    path
}

#[allow(dead_code)]
pub fn initialize_config() -> Result<(), String> {
    let path = get_config_path();
    let config = AnprConfig::load_from_file(&path)?;
    update_config(config);
    Ok(())
}
