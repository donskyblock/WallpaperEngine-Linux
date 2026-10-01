use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct ToolsStatus {
    pub steamcmd: bool,
    pub mpvpaper: bool,
    pub awww: bool,
    pub hyprpaper: bool,
    pub linux_wallpaperengine: bool,
    pub we_assets: bool,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Display {
    pub name: String,
    pub description: String,
    pub width: u32,
    pub height: u32,
    pub refresh_rate: f64,
    pub x: i32,
    pub y: i32,
    pub focused: bool,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct WorkshopItem {
    pub id: String,
    pub title: String,
    pub preview_url: String,
    pub description: String,
    pub tags: Vec<String>,
    pub subscriptions: u64,
    pub favorited: u64,
    pub votes_up: u64,
    pub votes_down: u64,
    pub score: f64,
    pub content_type: String,
    pub time_updated: u64,
    pub time_created: u64,
    pub file_size: u64,
    pub views: u64,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct WorkshopSearchResult {
    pub items: Vec<WorkshopItem>,
    pub total: u64,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct AppSettings {
    pub steam_api_key: String,
    pub download_dir: String,
    pub default_monitor: String,
    pub steam_username: String,
    pub audio_enabled: bool,
    pub fps_limit: u32,
}

impl Default for AppSettings {
    fn default() -> Self {
        let home = std::env::var("HOME").unwrap_or_else(|_| "/tmp".to_string());
        Self {
            steam_api_key: String::new(),
            download_dir: format!("{}/.local/share/wpe-linux/wallpapers", home),
            default_monitor: String::new(),
            steam_username: String::new(),
            audio_enabled: true,
            fps_limit: 30,
        }
    }
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct LibraryItem {
    pub id: String,
    pub title: String,
    pub path: String,
    pub preview_path: Option<String>,
    pub content_type: String,
    pub workshop_id: Option<String>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct WallpaperStatus {
    pub monitor: String,
    pub active: bool,
    pub current_path: Option<String>,
    pub content_type: Option<String>,
}
