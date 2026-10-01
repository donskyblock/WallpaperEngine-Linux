use crate::models::AppSettings;
use std::path::PathBuf;

#[tauri::command]
pub async fn needs_onboarding() -> bool {
    if std::env::args().any(|a| a == "--onboarding") {
        return true;
    }
    match load_settings().await {
        Ok(s) => s.steam_api_key.is_empty(),
        Err(_) => true,
    }
}

fn settings_path() -> PathBuf {
    let home = std::env::var("HOME").unwrap_or_else(|_| "/tmp".to_string());
    PathBuf::from(home)
        .join(".config")
        .join("wpe-linux")
        .join("settings.json")
}

#[tauri::command]
pub async fn load_settings() -> Result<AppSettings, String> {
    let path = settings_path();
    if !path.exists() {
        return Ok(AppSettings::default());
    }
    let data = std::fs::read_to_string(&path)
        .map_err(|e| format!("Failed to read settings: {}", e))?;
    serde_json::from_str(&data)
        .map_err(|e| format!("Failed to parse settings: {}", e))
}

#[tauri::command]
pub async fn save_settings(settings: AppSettings) -> Result<(), String> {
    let path = settings_path();
    if let Some(parent) = path.parent() {
        std::fs::create_dir_all(parent)
            .map_err(|e| format!("Failed to create config dir: {}", e))?;
    }
    let data = serde_json::to_string_pretty(&settings)
        .map_err(|e| format!("Failed to serialize settings: {}", e))?;
    std::fs::write(&path, data)
        .map_err(|e| format!("Failed to write settings: {}", e))
}
