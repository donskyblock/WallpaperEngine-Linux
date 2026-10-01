use std::path::Path;
use std::process::{Command, Stdio};
use std::sync::Mutex;
use std::collections::HashMap;

use crate::de::{self, DesktopEnvironment};
use crate::models::Display;

static ACTIVE_PIDS: Mutex<Option<HashMap<String, u32>>> = Mutex::new(None);

fn pids() -> std::sync::MutexGuard<'static, Option<HashMap<String, u32>>> {
    ACTIVE_PIDS.lock().unwrap()
}

fn tool_exists(name: &str) -> bool {
    let known_paths = ["/usr/bin", "/usr/local/bin", "/opt/bin"];
    for dir in &known_paths {
        if Path::new(dir).join(name).exists() {
            return true;
        }
    }
    Command::new("sh")
        .args(["-c", &format!("command -v {}", name)])
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .status()
        .map(|s| s.success())
        .unwrap_or(false)
}

fn ensure_awww_daemon() -> Result<(), String> {
    let running = Command::new("pgrep")
        .args(["-x", "awww-daemon"])
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .status()
        .map(|s| s.success())
        .unwrap_or(false);

    if !running {
        Command::new("awww-daemon")
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .spawn()
            .map_err(|e| format!("Failed to start awww-daemon: {}", e))?;
        std::thread::sleep(std::time::Duration::from_millis(600));
    }
    Ok(())
}

#[tauri::command]
pub async fn set_wallpaper(
    path: String,
    monitor: String,
    content_type: String,
) -> Result<(), String> {
    match content_type.to_lowercase().as_str() {
        "video" => set_video_wallpaper(path, monitor).await,
        "scene" => set_scene_wallpaper(path, monitor).await,
        _ => set_image_wallpaper(path, monitor).await,
    }
}

async fn set_image_wallpaper(path: String, monitor: String) -> Result<(), String> {
    match de::detect() {
        DesktopEnvironment::GNOME => set_image_gnome(path),
        _ => set_image_awww(path, monitor),
    }
}

fn set_image_awww(path: String, monitor: String) -> Result<(), String> {
    ensure_awww_daemon()?;

    let mut args = vec![
        "img".to_string(),
        path.clone(),
        "--transition-type".to_string(),
        "fade".to_string(),
        "--transition-duration".to_string(),
        "1".to_string(),
    ];

    if !monitor.is_empty() {
        args.push("--outputs".to_string());
        args.push(monitor.clone());
    }

    let out = Command::new("awww")
        .args(&args)
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .output()
        .map_err(|e| format!("Failed to run awww: {}", e))?;

    if !out.status.success() {
        let err = String::from_utf8_lossy(&out.stderr);
        return Err(format!("awww failed: {}", err.trim()));
    }
    Ok(())
}

fn set_image_gnome(path: String) -> Result<(), String> {
    let uri = format!("file://{}", path);
    let out = Command::new("gsettings")
        .args(["set", "org.gnome.desktop.background", "picture-uri", &uri])
        .output()
        .map_err(|e| format!("Failed to run gsettings: {}", e))?;
    if !out.status.success() {
        return Err(format!("gsettings failed: {}", String::from_utf8_lossy(&out.stderr).trim()));
    }
    // Also set dark variant
    let _ = Command::new("gsettings")
        .args(["set", "org.gnome.desktop.background", "picture-uri-dark", &uri])
        .output();
    let _ = Command::new("gsettings")
        .args(["set", "org.gnome.desktop.background", "picture-options", "zoom"])
        .output();
    Ok(())
}

pub fn find_we_assets() -> Option<String> {
    let home = std::env::var("HOME").unwrap_or_else(|_| "/tmp".to_string());
    let candidates = [
        format!("{home}/.local/share/wpe-linux/wallpaper_engine/assets"),
        format!("{home}/.steam/steam/steamapps/common/wallpaper_engine/assets"),
        format!("{home}/.local/share/Steam/steamapps/common/wallpaper_engine/assets"),
        format!("{home}/.steam/debian-installation/steamapps/common/wallpaper_engine/assets"),
        "/opt/linux-wallpaperengine/assets".to_string(),
    ];
    candidates.into_iter().find(|p| Path::new(p).is_dir())
}

async fn set_scene_wallpaper(path: String, monitor: String) -> Result<(), String> {
    // If the "scene" directory contains a plain video file but no scene.pkg,
    // skip lwe and go straight to mpvpaper — more reliable and avoids
    // crashes on wallpapers with scripted-property scene JSON.
    let scene_path = Path::new(&path);
    if scene_path.is_dir() && !scene_path.join("scene.pkg").exists() {
        let video_exts = ["mp4", "webm", "mkv", "avi", "mov"];
        let video_file = std::fs::read_dir(scene_path)
            .ok()
            .and_then(|entries| {
                entries
                    .filter_map(|e| e.ok())
                    .map(|e| e.path())
                    .find(|p| {
                        p.extension()
                            .and_then(|e| e.to_str())
                            .map(|e| video_exts.contains(&e.to_lowercase().as_str()))
                            .unwrap_or(false)
                    })
            });
        if let Some(vp) = video_file {
            return Box::pin(set_video_wallpaper(
                vp.to_string_lossy().to_string(),
                monitor,
            ))
            .await;
        }
    }

    if !tool_exists("linux-wallpaperengine") {
        return Err(
            "linux-wallpaperengine not installed — run: yay -S linux-wallpaperengine".to_string()
        );
    }

    let assets = find_we_assets().ok_or_else(|| {
        "Wallpaper Engine assets not found. Install WE via Steam (steam://install/431960)".to_string()
    })?;

    stop_wallpaper_on_monitor(monitor.clone())?;

    let mon = if monitor.is_empty() {
        get_primary_monitor()?
    } else {
        monitor.clone()
    };

    match de::detect() {
        DesktopEnvironment::GNOME => set_scene_gnome(path, mon, assets),
        DesktopEnvironment::KDE => {
            let info = get_monitor_geometry(&mon);
            let result = set_scene_layer_shell(path, mon.clone(), assets);
            if result.is_ok() {
                if let Some(info) = info {
                    spawn_kwin_window_enforcer("linux-wallpaperengine", &info);
                }
            }
            result
        }
        _ => set_scene_layer_shell(path, mon, assets),
    }
}

fn set_scene_layer_shell(path: String, mon: String, assets: String) -> Result<(), String> {
    use std::io::Read;

    let mut child = Command::new("linux-wallpaperengine")
        // --disable-mouse: layer surface won't eat clicks, they fall through to the desktop
        // --silent: suppress lwe's own logging
        .args([
            "--assets-dir", &assets,
            "--screen-root", &mon,
            "--bg", &path,
            "--silent",
            "--disable-mouse",
        ])
        // Force C locale so std::stof() parses JSON floats correctly regardless of system locale
        .env("LC_ALL", "C")
        .stdout(Stdio::null())
        .stderr(Stdio::piped())
        .spawn()
        .map_err(|e| format!("Failed to spawn linux-wallpaperengine: {}", e))?;

    // Give lwe a moment to initialise — scene parse crashes happen within ~500ms
    std::thread::sleep(std::time::Duration::from_millis(800));

    match child.try_wait() {
        Ok(Some(status)) => {
            let mut stderr_buf = String::new();
            if let Some(mut se) = child.stderr.take() {
                let _ = se.read_to_string(&mut stderr_buf);
            }
            let detail = extract_lwe_error(&stderr_buf);
            Err(format!("linux-wallpaperengine crashed: {}", detail))
        }
        Ok(None) => {
            // Still running — wallpaper is up
            store_pid(mon, child.id());
            Ok(())
        }
        Err(e) => Err(format!("Failed to check lwe process: {}", e)),
    }
}

fn set_scene_gnome(path: String, mon: String, assets: String) -> Result<(), String> {
    let display_info = get_monitor_geometry(&mon);

    // Force XWayland so wmctrl/xdotool can manage the window
    let mut child = Command::new("linux-wallpaperengine")
        .env("WAYLAND_DISPLAY", "")
        .env("DISPLAY", ":0")
        .env("SDL_VIDEODRIVER", "x11")
        .env("GDK_BACKEND", "x11")
        .env("LC_ALL", "C")
        .args(["--assets-dir", &assets, "--bg", &path, "--silent", "--disable-mouse"])
        .stdout(Stdio::null())
        .stderr(Stdio::piped())
        .spawn()
        .map_err(|e| format!("Failed to spawn linux-wallpaperengine: {}", e))?;

    use std::io::Read;
    std::thread::sleep(std::time::Duration::from_millis(800));

    match child.try_wait() {
        Ok(Some(status)) => {
            let mut buf = String::new();
            if let Some(mut se) = child.stderr.take() {
                let _ = se.read_to_string(&mut buf);
            }
            let detail = extract_lwe_error(&buf);
            Err(format!("linux-wallpaperengine crashed: {}", detail))
        }
        Ok(None) => {
            let pid = child.id();
            store_pid(mon, pid);
            if let Some(info) = display_info {
                apply_window_rules_xdotool(pid, &info);
            }
            Ok(())
        }
        Err(e) => Err(format!("Failed to check lwe process: {}", e)),
    }
}

async fn set_video_wallpaper(path: String, monitor: String) -> Result<(), String> {
    stop_wallpaper_on_monitor(monitor.clone())?;

    let mon = if monitor.is_empty() {
        get_primary_monitor()?
    } else {
        monitor.clone()
    };

    match de::detect() {
        DesktopEnvironment::GNOME => set_video_gnome(path, mon),
        DesktopEnvironment::KDE => {
            let info = get_monitor_geometry(&mon);
            let result = set_video_mpvpaper(path, mon.clone());
            if result.is_ok() {
                if let Some(info) = info {
                    spawn_kwin_window_enforcer("mpvpaper", &info);
                }
            }
            result
        }
        _ => set_video_mpvpaper(path, mon),
    }
}

fn set_video_mpvpaper(path: String, mon: String) -> Result<(), String> {
    if !tool_exists("mpvpaper") {
        return Err("mpvpaper not installed. Run: yay -S mpvpaper".to_string());
    }

    let child = Command::new("mpvpaper")
        .args(["-o", "--loop --no-audio --hwdec=auto", &mon, &path])
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("Failed to spawn mpvpaper: {}", e))?;

    store_pid(mon, child.id());
    Ok(())
}

fn set_video_gnome(path: String, mon: String) -> Result<(), String> {
    if !tool_exists("mpv") {
        return Err(
            "mpv not installed. Run: yay -S mpv (required for video wallpapers on GNOME)".to_string()
        );
    }

    let display_info = get_monitor_geometry(&mon);

    let geometry_arg = display_info.as_ref().map(|d| {
        format!("{}x{}+{}+{}", d.width, d.height, d.x, d.y)
    }).unwrap_or_else(|| "1920x1080+0+0".to_string());

    // Run mpv via XWayland so wmctrl/xdotool can apply window rules
    let child = Command::new("mpv")
        .env("WAYLAND_DISPLAY", "")
        .env("DISPLAY", ":0")
        .args([
            "--loop",
            "--no-audio",
            "--hwdec=auto",
            "--no-border",
            "--no-osc",
            "--no-osd-bar",
            "--x11-name=wpe-wallpaper",
            "--title=wpe-wallpaper",
            "--geometry", &geometry_arg,
            &path,
        ])
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("Failed to spawn mpv: {}", e))?;

    let pid = child.id();
    store_pid(mon, pid);

    if let Some(info) = display_info {
        apply_window_rules_xdotool(pid, &info);
    }
    Ok(())
}

// Writes a KWin JS script that enforces borderless/keepbelow/fullscreen on the target window class,
// then loads it via D-Bus. Works for both layer-shell windows and regular toplevels on KDE Plasma.
fn spawn_kwin_window_enforcer(window_class: &str, monitor: &Display) {
    let cls = window_class.to_string();
    let x = monitor.x;
    let y = monitor.y;
    let w = monitor.width;
    let h = monitor.height;

    let script = format!(
        r#"(function() {{
  var cls = "{cls}";
  function enforce(win) {{
    if (win.resourceClass.toLowerCase().indexOf(cls) < 0 &&
        win.resourceName.toLowerCase().indexOf(cls) < 0) return;
    win.keepBelow = true;
    win.noBorder = true;
    win.skipTaskbar = true;
    win.skipSwitcher = true;
    win.onAllDesktops = true;
    // Prevent it from stealing focus when it first appears
    win.minimizable = false;
    win.maximizable = false;
    win.frameGeometry = Qt.rect({x}, {y}, {w}, {h});
  }}
  workspace.windowAdded.connect(enforce);
  // Also apply to any already-running instance
  var wins = workspace.windowList();
  for (var i = 0; i < wins.length; i++) enforce(wins[i]);
}})();
"#
    );

    let script_path = format!("/tmp/wpe_kwin_{}.js", cls.replace('-', "_"));
    let script_name = format!("wpe-{}", cls);

    if std::fs::write(&script_path, script).is_err() {
        return;
    }

    // Unload previous version so we always get the updated geometry
    let _ = Command::new("dbus-send")
        .args([
            "--session", "--dest=org.kde.KWin",
            "/Scripting",
            "org.kde.kwin.Scripting.unloadScript",
            &format!("string:{}", script_name),
        ])
        .status();

    let _ = Command::new("dbus-send")
        .args([
            "--session", "--dest=org.kde.KWin",
            "/Scripting",
            "org.kde.kwin.Scripting.loadScript",
            &format!("string:{}", script_path),
            &format!("string:{}", script_name),
        ])
        .status();
}

// Uses xdotool to find the window by PID and applies "below + sticky" rules via wmctrl.
// Runs in a background thread so we don't block the caller while waiting for the window.
fn apply_window_rules_xdotool(pid: u32, monitor: &Display) {
    let x = monitor.x;
    let y = monitor.y;
    let w = monitor.width;
    let h = monitor.height;

    std::thread::spawn(move || {
        // Give the process time to create its window
        std::thread::sleep(std::time::Duration::from_millis(1500));

        let xid = Command::new("xdotool")
            .env("DISPLAY", ":0")
            .args(["search", "--pid", &pid.to_string()])
            .output()
            .ok()
            .and_then(|o| {
                String::from_utf8(o.stdout).ok().and_then(|s| {
                    s.split_whitespace().next_back().map(|s| s.to_string())
                })
            });

        if let Some(xid) = xid {
            // Size and position the window over the target monitor
            let _ = Command::new("xdotool")
                .env("DISPLAY", ":0")
                .args(["windowsize", &xid, &w.to_string(), &h.to_string()])
                .status();
            let _ = Command::new("xdotool")
                .env("DISPLAY", ":0")
                .args(["windowmove", &xid, &x.to_string(), &y.to_string()])
                .status();

            // Push below all windows and make sticky (visible on all virtual desktops)
            let _ = Command::new("wmctrl")
                .env("DISPLAY", ":0")
                .args(["-i", "-r", &xid, "-b", "add,below,sticky"])
                .status();

            // Pin to desktop 0 so it survives workspace switches
            let _ = Command::new("xdotool")
                .env("DISPLAY", ":0")
                .args(["set_desktop_for_window", &xid, "0"])
                .status();
        }
    });
}

fn extract_lwe_error(stderr: &str) -> String {
    if stderr.trim().is_empty() {
        return "no output — likely a Vulkan/EGL init failure. Try: yay -Syu linux-wallpaperengine".to_string();
    }
    // Pull out the most informative line: the json exception, a "Cannot find" line,
    // a "filesystem error", or failing that the last non-empty line.
    let priority_line = stderr.lines().rev().find(|l| {
        let l = l.trim();
        !l.is_empty() && (
            l.contains("what():") ||
            l.contains("json.exception") ||
            l.contains("Cannot find") ||
            l.contains("filesystem error") ||
            l.contains("Failed to") ||
            l.contains("terminate called")
        )
    });
    if let Some(line) = priority_line {
        // Strip the verbose nlohmann prefix if present
        let msg = line.trim()
            .trim_start_matches("terminate called after throwing an instance of ")
            .trim_start_matches('\'')
            .trim_end_matches('\'');
        return msg.to_string();
    }
    // Fallback: last non-empty line
    stderr.lines().rev().find(|l| !l.trim().is_empty())
        .unwrap_or("unknown error").trim().to_string()
}

fn get_primary_monitor() -> Result<String, String> {
    let monitors = crate::commands::displays::list_monitors()
        .map_err(|e| format!("Could not enumerate monitors: {}", e))?;

    monitors
        .into_iter()
        .find(|m| m.focused)
        .or_else(|| {
            crate::commands::displays::list_monitors().ok()?.into_iter().next()
        })
        .map(|m| m.name)
        .ok_or_else(|| "No monitors found".to_string())
}

fn get_monitor_geometry(monitor: &str) -> Option<Display> {
    crate::commands::displays::list_monitors()
        .ok()?
        .into_iter()
        .find(|d| d.name == monitor)
}

fn store_pid(monitor: String, pid: u32) {
    let mut guard = pids();
    if guard.is_none() {
        *guard = Some(HashMap::new());
    }
    if let Some(map) = guard.as_mut() {
        map.insert(monitor, pid);
    }
}

fn stop_wallpaper_on_monitor(monitor: String) -> Result<(), String> {
    let mut guard = pids();
    if let Some(map) = guard.as_mut() {
        if let Some(&pid) = map.get(&monitor) {
            let _ = Command::new("kill")
                .arg(pid.to_string())
                .stdout(Stdio::null())
                .stderr(Stdio::null())
                .status();
            map.remove(&monitor);
        }
    }
    Ok(())
}

#[tauri::command]
pub async fn stop_wallpaper(monitor: String) -> Result<(), String> {
    stop_wallpaper_on_monitor(monitor)?;
    for proc in &["mpvpaper", "mpv", "linux-wallpaperengine"] {
        let _ = Command::new("pkill")
            .args(["-f", proc])
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .status();
    }
    Ok(())
}

pub fn kill_all_wallpapers() {
    let mut guard = pids();
    if let Some(map) = guard.as_mut() {
        for &pid in map.values() {
            let _ = Command::new("kill")
                .arg(pid.to_string())
                .stdout(Stdio::null())
                .stderr(Stdio::null())
                .status();
        }
        map.clear();
    }
    for proc in &["mpvpaper", "mpv", "linux-wallpaperengine"] {
        let _ = Command::new("pkill")
            .args(["-f", proc])
            .stdout(Stdio::null())
            .stderr(Stdio::null())
            .status();
    }
}

#[tauri::command]
pub async fn get_wallpaper_status() -> Result<Vec<crate::models::WallpaperStatus>, String> {
    let monitors = crate::commands::displays::list_monitors().unwrap_or_default();
    let guard = pids();
    let active_map = guard.as_ref();

    Ok(monitors
        .into_iter()
        .map(|m| {
            let active = active_map.and_then(|map| map.get(&m.name)).is_some();
            crate::models::WallpaperStatus {
                monitor: m.name.clone(),
                active,
                current_path: None,
                content_type: None,
            }
        })
        .collect())
}
