use std::io::{BufRead, BufReader};
use std::process::{Command, Stdio};
use std::path::Path;
use crate::models::ToolsStatus;

#[derive(Clone, serde::Serialize)]
struct DownloadProgress {
    item_id: String,
    line: String,
}

fn strip_ansi(s: &str) -> String {
    let mut out = String::with_capacity(s.len());
    let mut chars = s.chars().peekable();
    while let Some(c) = chars.next() {
        if c == '\x1b' {
            // skip ESC [ ... <letter>
            if chars.peek() == Some(&'[') {
                chars.next();
                for ch in chars.by_ref() {
                    if ch.is_ascii_alphabetic() { break; }
                }
            }
        } else {
            out.push(c);
        }
    }
    out
}

fn detect_terminal() -> Option<(String, Vec<String>)> {
    // Check $TERMINAL env var first
    if let Ok(term) = std::env::var("TERMINAL") {
        if tool_exists_path(&term) {
            return Some((term, vec!["-e".to_string()]));
        }
    }

    let candidates: &[(&str, &[&str])] = &[
        ("foot",      &["-e"]),
        ("kitty",     &["--"]),
        ("ghostty",   &["-e"]),
        ("alacritty", &["-e"]),
        ("wezterm",   &["start", "--"]),
        ("xterm",     &["-e"]),
        ("urxvt",     &["-e"]),
    ];

    for (name, args) in candidates {
        if tool_exists(name) {
            return Some((name.to_string(), args.iter().map(|s| s.to_string()).collect()));
        }
    }
    None
}

fn tool_exists_path(name: &str) -> bool {
    std::path::Path::new(name).exists() || tool_exists(name)
}

#[tauri::command]
pub async fn open_steamcmd_login(username: String) -> Result<(), String> {
    if !tool_exists("steamcmd") {
        return Err("steamcmd not installed. Run: yay -S steamcmd".to_string());
    }

    let (term, mut args) = detect_terminal()
        .ok_or_else(|| "No terminal emulator found (tried: foot, kitty, ghostty, alacritty, wezterm, xterm)".to_string())?;

    // Run steamcmd login and keep window open so user can see result
    let cmd = if username.is_empty() {
        "steamcmd +login anonymous +quit; echo '--- Done. Press Enter to close ---'; read".to_string()
    } else {
        format!(
            "steamcmd +login {} +quit; echo '--- Done. Press Enter to close ---'; read",
            username
        )
    };

    args.push("sh".to_string());
    args.push("-c".to_string());
    args.push(cmd);

    Command::new(&term)
        .args(&args)
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("Failed to open {}: {}", term, e))?;

    Ok(())
}

#[tauri::command]
pub async fn download_we_assets(username: String) -> Result<String, String> {
    if !tool_exists("steamcmd") {
        return Err("steamcmd not installed. Run: yay -S steamcmd".to_string());
    }
    if username.is_empty() {
        return Err("Steam username required to download Wallpaper Engine.".to_string());
    }

    let (term, mut args) = detect_terminal()
        .ok_or_else(|| "No terminal emulator found".to_string())?;

    let home = std::env::var("HOME").unwrap_or_else(|_| "/tmp".to_string());
    let install_dir = format!("{}/.local/share/wpe-linux/wallpaper_engine", home);
    std::fs::create_dir_all(&install_dir)
        .map_err(|e| format!("Failed to create dir: {}", e))?;

    let cmd = format!(
        "echo 'Downloading Wallpaper Engine assets via steamcmd...' && \
         steamcmd +force_install_dir '{}' +login {} +app_update 431960 validate +quit; \
         echo '--- Done. Press Enter to close ---'; read",
        install_dir, username
    );

    args.push("sh".to_string());
    args.push("-c".to_string());
    args.push(cmd);

    Command::new(&term)
        .args(&args)
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("Failed to open terminal: {}", e))?;

    Ok(install_dir)
}

pub fn tool_exists(name: &str) -> bool {
    ["/usr/bin", "/usr/local/bin", "/opt/bin"]
        .iter()
        .any(|dir| Path::new(dir).join(name).exists())
        || Command::new("sh")
            .args(["-c", &format!("command -v {}", name)])
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .status()
            .map(|s| s.success())
            .unwrap_or(false)
}

fn default_download_dir() -> String {
    let home = std::env::var("HOME").unwrap_or_else(|_| "/tmp".to_string());
    format!("{}/.local/share/wpe-linux/wallpapers", home)
}

#[tauri::command]
pub async fn check_steamcmd() -> bool {
    tool_exists("steamcmd")
}

#[tauri::command]
pub async fn check_tools() -> ToolsStatus {
    ToolsStatus {
        steamcmd: tool_exists("steamcmd"),
        mpvpaper: tool_exists("mpvpaper"),
        awww: tool_exists("awww"),
        hyprpaper: tool_exists("hyprpaper"),
        linux_wallpaperengine: tool_exists("linux-wallpaperengine"),
        we_assets: crate::commands::wallpaper::find_we_assets().is_some(),
    }
}

#[tauri::command]
pub async fn download_workshop_item(
    app: tauri::AppHandle,
    item_id: String,
    steam_username: String,
    download_dir: String,
) -> Result<String, String> {
    use tauri::Emitter;

    if !tool_exists("steamcmd") {
        return Err("steamcmd not installed. Run: yay -S steamcmd".to_string());
    }

    if steam_username.is_empty() {
        return Err(
            "Steam username required — Wallpaper Engine workshop items need account login. Add it in Settings.".to_string()
        );
    }

    let dir = if download_dir.is_empty() {
        default_download_dir()
    } else {
        download_dir.clone()
    };

    std::fs::create_dir_all(&dir)
        .map_err(|e| format!("Failed to create download dir: {}", e))?;

    let mut child = Command::new("steamcmd")
        .args([
            "+force_install_dir", &dir,
            "+login", &steam_username,
            "+workshop_download_item", "431960", &item_id,
            "+quit",
        ])
        .stdout(Stdio::piped())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("Failed to launch steamcmd: {}", e))?;

    // Stream stdout line-by-line, emit events, and collect for final error check.
    // BufReader::lines() splits on \n — steamcmd's key status lines (login, success, errors)
    // are all \n-terminated; \r-only progress bars are intentionally skipped.
    let mut collected: Vec<String> = Vec::new();
    if let Some(stdout) = child.stdout.take() {
        let reader = BufReader::new(stdout);
        for line in reader.lines() {
            if let Ok(raw) = line {
                let clean = strip_ansi(raw.trim_end_matches('\r'));
                let s = clean.trim().to_string();
                if !s.is_empty() {
                    let _ = app.emit("download-progress", DownloadProgress {
                        item_id: item_id.clone(),
                        line: s.clone(),
                    });
                    collected.push(s);
                }
            }
        }
    }

    child.wait().map_err(|e| format!("steamcmd wait failed: {}", e))?;

    let stdout = collected.join("\n");

    if let Some(error_line) = collected.iter().find(|l| l.contains("ERROR")) {
        let msg = error_line.trim();
        if msg.contains("Invalid Password") {
            return Err(
                "steamcmd: Invalid Password — run 'steamcmd +login <username> +quit' in a terminal first to cache credentials, then retry.".to_string()
            );
        }
        if msg.contains("Not logged on") {
            return Err(
                "steamcmd: Not logged on — run 'steamcmd +login <username> +quit' in a terminal to authenticate, then retry.".to_string()
            );
        }
        return Err(format!("steamcmd: {}", msg));
    }

    if stdout.contains("Timeout downloading item") {
        return Err("Download timed out. Check your internet connection.".to_string());
    }

    if !stdout.contains("Success") {
        let snippet: String = collected.iter().rev().take(3).cloned().collect::<Vec<_>>().join(" | ");
        return Err(format!("steamcmd failed: {}", snippet));
    }

    let item_path = Path::new(&dir)
        .join("steamapps")
        .join("workshop")
        .join("content")
        .join("431960")
        .join(&item_id);

    if item_path.exists() {
        Ok(item_path.to_string_lossy().to_string())
    } else {
        Err("Download may have succeeded but files not found at expected path.".to_string())
    }
}

#[tauri::command]
pub async fn scan_library(download_dir: String) -> Result<Vec<crate::models::LibraryItem>, String> {
    let dir = if download_dir.is_empty() {
        default_download_dir()
    } else {
        download_dir.clone()
    };

    let base = Path::new(&dir)
        .join("steamapps")
        .join("workshop")
        .join("content")
        .join("431960");

    let mut items = Vec::new();

    if base.exists() {
        for entry in std::fs::read_dir(&base)
            .map_err(|e| format!("Failed to read library dir: {}", e))?
        {
            let entry = entry.map_err(|e| e.to_string())?;
            let path = entry.path();
            if !path.is_dir() { continue; }

            let item_id = path.file_name().unwrap().to_string_lossy().to_string();
            let (title, content_type, file_path, preview_path) = read_workshop_item_meta(&path);

            if let Some(fp) = file_path {
                items.push(crate::models::LibraryItem {
                    id: uuid_from_path(&fp),
                    title,
                    path: fp,
                    preview_path,
                    content_type,
                    workshop_id: Some(item_id),
                });
            }
        }
    }

    let local = scan_local_dir(&dir).unwrap_or_default();
    items.extend(local);

    Ok(items)
}

fn read_workshop_item_meta(dir: &Path) -> (String, String, Option<String>, Option<String>) {
    let project_json = dir.join("project.json");
    let mut title = dir.file_name().unwrap().to_string_lossy().to_string();
    let mut content_type = "unknown".to_string();
    let mut preview_path = None;

    if project_json.exists() {
        if let Ok(data) = std::fs::read_to_string(&project_json) {
            if let Ok(v) = serde_json::from_str::<serde_json::Value>(&data) {
                if let Some(t) = v["title"].as_str() { title = t.to_string(); }
                if let Some(ct) = v["type"].as_str() { content_type = ct.to_string(); }
                if let Some(p) = v["preview"].as_str() {
                    preview_path = Some(dir.join(p).to_string_lossy().to_string());
                }
            }
        }
    }

    let preview_fallback = ["preview.jpg", "preview.png", "preview.gif"]
        .iter()
        .find_map(|name| {
            let p = dir.join(name);
            if p.exists() { Some(p.to_string_lossy().to_string()) } else { None }
        });

    let preview = preview_path.or(preview_fallback);
    let file = find_main_file(dir, &content_type);
    (title, content_type, file, preview)
}

fn find_main_file(dir: &Path, content_type: &str) -> Option<String> {
    let video_exts = ["mp4", "webm", "mkv", "avi", "mov"];
    let image_exts = ["jpg", "jpeg", "png", "gif", "webp", "bmp"];

    let ct = content_type.to_lowercase();

    if ct == "web" {
        let web_file = dir.join("index.html");
        if web_file.exists() {
            return Some(web_file.to_string_lossy().to_string());
        }
    }

    // Scene: linux-wallpaperengine takes the directory itself.
    // Detect by scene.pkg presence or project.json type field.
    if ct == "scene" || dir.join("scene.pkg").exists() {
        return Some(dir.to_string_lossy().to_string());
    }

    let all_files: Vec<_> = std::fs::read_dir(dir)
        .ok()?
        .filter_map(|e| e.ok())
        .map(|e| e.path())
        .collect();

    for f in &all_files {
        if let Some(ext) = f.extension().and_then(|e| e.to_str()) {
            if video_exts.contains(&ext.to_lowercase().as_str()) {
                return Some(f.to_string_lossy().to_string());
            }
        }
    }

    for f in &all_files {
        if let Some(ext) = f.extension().and_then(|e| e.to_str()) {
            if image_exts.contains(&ext.to_lowercase().as_str()) {
                return Some(f.to_string_lossy().to_string());
            }
        }
    }

    None
}

fn scan_local_dir(dir: &str) -> Result<Vec<crate::models::LibraryItem>, String> {
    let path = Path::new(dir);
    if !path.exists() { return Ok(vec![]); }

    let video_exts = ["mp4", "webm", "mkv", "avi"];
    let image_exts = ["jpg", "jpeg", "png", "gif", "webp"];

    let items = std::fs::read_dir(path)
        .map_err(|e| e.to_string())?
        .filter_map(|e| e.ok())
        .filter_map(|entry| {
            let p = entry.path();
            if !p.is_file() { return None; }
            let ext = p.extension()?.to_str()?.to_lowercase();
            let content_type = if video_exts.contains(&ext.as_str()) {
                "video"
            } else if image_exts.contains(&ext.as_str()) {
                "scene"
            } else {
                return None;
            };
            let fname = p.file_stem()?.to_string_lossy().to_string();
            Some(crate::models::LibraryItem {
                id: uuid_from_path(&p.to_string_lossy()),
                title: fname,
                path: p.to_string_lossy().to_string(),
                preview_path: None,
                content_type: content_type.to_string(),
                workshop_id: None,
            })
        })
        .collect();

    Ok(items)
}

fn uuid_from_path(path: &str) -> String {
    use std::collections::hash_map::DefaultHasher;
    use std::hash::{Hash, Hasher};
    let mut h = DefaultHasher::new();
    path.hash(&mut h);
    format!("{:x}", h.finish())
}
