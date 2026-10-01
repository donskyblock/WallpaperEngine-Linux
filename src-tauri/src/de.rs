#[derive(Debug, Clone, PartialEq)]
pub enum DesktopEnvironment {
    Hyprland,
    KDE,
    GNOME,
    Unknown,
}

pub fn detect() -> DesktopEnvironment {
    if std::env::var("HYPRLAND_INSTANCE_SIGNATURE").is_ok() {
        return DesktopEnvironment::Hyprland;
    }
    let xdg = std::env::var("XDG_CURRENT_DESKTOP")
        .unwrap_or_default()
        .to_uppercase();
    if xdg.contains("KDE") {
        return DesktopEnvironment::KDE;
    }
    if xdg.contains("GNOME") {
        return DesktopEnvironment::GNOME;
    }
    let session = std::env::var("DESKTOP_SESSION")
        .unwrap_or_default()
        .to_lowercase();
    if session.contains("plasma") || session.contains("kde") {
        return DesktopEnvironment::KDE;
    }
    if session.contains("gnome") || session.contains("ubuntu") {
        return DesktopEnvironment::GNOME;
    }
    DesktopEnvironment::Unknown
}
