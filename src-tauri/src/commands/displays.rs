use crate::de::{self, DesktopEnvironment};
use crate::models::Display;
use std::process::Command;

#[tauri::command]
pub async fn get_displays() -> Result<Vec<Display>, String> {
    list_monitors()
}

pub fn list_monitors() -> Result<Vec<Display>, String> {
    match de::detect() {
        DesktopEnvironment::Hyprland => list_hyprland(),
        DesktopEnvironment::KDE => list_kscreen_doctor()
            .or_else(|_| list_wlr_randr())
            .or_else(|_| list_xrandr()),
        DesktopEnvironment::GNOME => list_xrandr(),
        DesktopEnvironment::Unknown => list_hyprland()
            .or_else(|_| list_kscreen_doctor())
            .or_else(|_| list_wlr_randr())
            .or_else(|_| list_xrandr()),
    }
}

fn list_hyprland() -> Result<Vec<Display>, String> {
    let output = Command::new("hyprctl")
        .args(["monitors", "-j"])
        .output()
        .map_err(|e| format!("Failed to run hyprctl: {}", e))?;

    if !output.status.success() {
        return Err("hyprctl monitors failed".to_string());
    }

    #[derive(serde::Deserialize)]
    struct HyprMonitor {
        name: String,
        description: String,
        width: u32,
        height: u32,
        #[serde(rename = "refreshRate")]
        refresh_rate: f64,
        x: i32,
        y: i32,
        focused: bool,
    }

    let monitors: Vec<HyprMonitor> =
        serde_json::from_str(&String::from_utf8_lossy(&output.stdout))
            .map_err(|e| format!("Failed to parse hyprctl output: {}", e))?;

    Ok(monitors
        .into_iter()
        .map(|m| Display {
            name: m.name,
            description: m.description,
            width: m.width,
            height: m.height,
            refresh_rate: m.refresh_rate,
            x: m.x,
            y: m.y,
            focused: m.focused,
        })
        .collect())
}

// kscreen-doctor is KDE's native tool — always present on KDE
fn list_kscreen_doctor() -> Result<Vec<Display>, String> {
    let output = Command::new("kscreen-doctor")
        .arg("-j")
        .output()
        .map_err(|e| format!("Failed to run kscreen-doctor: {}", e))?;

    if !output.status.success() {
        return Err("kscreen-doctor -j failed".to_string());
    }

    #[derive(serde::Deserialize)]
    struct KMode {
        id: String,
        #[serde(rename = "refreshRate")]
        refresh_rate: f64,
        size: KSize,
    }

    #[derive(serde::Deserialize)]
    struct KSize {
        width: u32,
        height: u32,
    }

    #[derive(serde::Deserialize)]
    struct KPos {
        x: i32,
        y: i32,
    }

    #[derive(serde::Deserialize)]
    struct KOutput {
        name: String,
        enabled: bool,
        #[serde(rename = "currentModeId")]
        current_mode_id: String,
        modes: Vec<KMode>,
        pos: KPos,
    }

    #[derive(serde::Deserialize)]
    struct KRoot {
        outputs: Vec<KOutput>,
    }

    let root: KRoot = serde_json::from_str(&String::from_utf8_lossy(&output.stdout))
        .map_err(|e| format!("Failed to parse kscreen-doctor output: {}", e))?;

    let displays: Vec<Display> = root
        .outputs
        .into_iter()
        .filter(|o| o.enabled)
        .filter_map(|o| {
            let mode = o.modes.iter().find(|m| m.id == o.current_mode_id)?;
            Some(Display {
                name: o.name.clone(),
                description: o.name,
                width: mode.size.width,
                height: mode.size.height,
                refresh_rate: mode.refresh_rate,
                x: o.pos.x,
                y: o.pos.y,
                focused: false,
            })
        })
        .collect();

    if displays.is_empty() {
        return Err("No enabled outputs from kscreen-doctor".to_string());
    }
    Ok(displays)
}

// wlr-randr works on KDE Plasma 5.27+ (wlr-output-management protocol)
fn list_wlr_randr() -> Result<Vec<Display>, String> {
    let output = Command::new("wlr-randr")
        .output()
        .map_err(|e| format!("Failed to run wlr-randr: {}", e))?;

    if !output.status.success() {
        return Err("wlr-randr failed".to_string());
    }

    let text = String::from_utf8_lossy(&output.stdout);
    parse_wlr_randr(&text)
}

fn parse_wlr_randr(text: &str) -> Result<Vec<Display>, String> {
    let mut displays: Vec<Display> = Vec::new();
    let mut name = String::new();
    let mut desc = String::new();
    let mut width = 0u32;
    let mut height = 0u32;
    let mut refresh = 0.0f64;
    let mut x = 0i32;
    let mut y = 0i32;
    let mut enabled = false;
    let mut in_output = false;

    for line in text.lines() {
        let is_header = !line.starts_with(' ') && !line.starts_with('\t') && !line.is_empty();

        if is_header {
            if in_output && enabled && width > 0 {
                displays.push(Display { name: name.clone(), description: desc.clone(), width, height, refresh_rate: refresh, x, y, focused: false });
            }
            // "OUTPUT_NAME "DESCRIPTION" (flags)"
            let mut parts = line.splitn(2, ' ');
            name = parts.next().unwrap_or("").to_string();
            desc = parts
                .next()
                .and_then(|rest| rest.split('"').nth(1))
                .unwrap_or("")
                .to_string();
            width = 0; height = 0; refresh = 0.0; x = 0; y = 0; enabled = false;
            in_output = true;
            continue;
        }

        let t = line.trim();

        // Current mode line: "1920x1080 60.000 Hz (preferred, current)"
        if t.contains("current") {
            let parts: Vec<&str> = t.split_whitespace().collect();
            if let Some(res) = parts.first() {
                if let Some((ws, hs)) = res.split_once('x') {
                    width = ws.parse().unwrap_or(0);
                    height = hs.parse().unwrap_or(0);
                }
            }
            if parts.len() >= 2 {
                refresh = parts[1].parse().unwrap_or(0.0);
            }
        }

        if t.starts_with("Enabled:") {
            enabled = t.contains("yes");
        }

        // "Position: X,Y"
        if let Some(pos) = t.strip_prefix("Position:") {
            let pos = pos.trim();
            if let Some((xs, ys)) = pos.split_once(',') {
                x = xs.trim().parse().unwrap_or(0);
                y = ys.trim().parse().unwrap_or(0);
            }
        }
    }

    if in_output && enabled && width > 0 {
        displays.push(Display { name, description: desc, width, height, refresh_rate: refresh, x, y, focused: false });
    }

    if displays.is_empty() {
        return Err("No enabled displays found via wlr-randr".to_string());
    }
    Ok(displays)
}

// xrandr via XWayland — works on GNOME (which always runs XWayland)
fn list_xrandr() -> Result<Vec<Display>, String> {
    let output = Command::new("xrandr")
        .env("DISPLAY", ":0")
        .output()
        .map_err(|e| format!("Failed to run xrandr: {}", e))?;

    if !output.status.success() {
        return Err("xrandr failed".to_string());
    }

    let text = String::from_utf8_lossy(&output.stdout);
    parse_xrandr(&text)
}

fn parse_xrandr(text: &str) -> Result<Vec<Display>, String> {
    let mut displays: Vec<Display> = Vec::new();
    let mut lines = text.lines().peekable();

    while let Some(line) = lines.next() {
        if !line.contains(" connected ") {
            continue;
        }

        let parts: Vec<&str> = line.split_whitespace().collect();
        if parts.len() < 3 {
            continue;
        }
        let name = parts[0].to_string();

        let geometry_str = parts.iter().find(|p| {
            p.contains('x') && p.contains('+')
        });
        let (w, h, ox, oy) = match geometry_str.and_then(|g| parse_xrandr_geometry(g)) {
            Some(v) => v,
            None => continue,
        };

        let mut refresh = 0.0f64;
        while let Some(&next) = lines.peek() {
            if !next.starts_with(' ') && !next.starts_with('\t') {
                break;
            }
            let next = lines.next().unwrap();
            if next.contains('*') {
                let mode_parts: Vec<&str> = next.split_whitespace().collect();
                // mode_parts[0] = "WxH", rest are refresh rates; * marks current
                for part in mode_parts.iter().skip(1) {
                    let rate = part.trim_matches(|c| c == '*' || c == '+');
                    if let Ok(r) = rate.parse::<f64>() {
                        refresh = r;
                        break;
                    }
                }
                break;
            }
        }

        displays.push(Display {
            name: name.clone(),
            description: name,
            width: w,
            height: h,
            refresh_rate: refresh,
            x: ox,
            y: oy,
            focused: false,
        });
    }

    if displays.is_empty() {
        return Err("No connected displays found via xrandr".to_string());
    }
    Ok(displays)
}

fn parse_xrandr_geometry(s: &str) -> Option<(u32, u32, i32, i32)> {
    // "1920x1080+0+0"
    let (res, rest) = s.split_once('+')?;
    let (xs, ys) = rest.split_once('+')?;
    let (ws, hs) = res.split_once('x')?;
    Some((ws.parse().ok()?, hs.parse().ok()?, xs.parse().ok()?, ys.parse().ok()?))
}
