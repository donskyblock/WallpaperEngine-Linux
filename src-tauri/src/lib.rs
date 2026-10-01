mod commands;
mod de;
mod models;

use commands::{
    displays::get_displays,
    downloader::{check_steamcmd, check_tools, download_we_assets, download_workshop_item, open_steamcmd_login, scan_library},
    settings::{load_settings, needs_onboarding, save_settings},
    wallpaper::{get_wallpaper_status, kill_all_wallpapers, set_wallpaper, stop_wallpaper},
    workshop::{get_workshop_item, search_workshop},
};
use tauri::menu::{Menu, MenuItem, PredefinedMenuItem};
use tauri::tray::{MouseButton, MouseButtonState, TrayIconBuilder, TrayIconEvent};
use tauri::Manager;

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .plugin(tauri_plugin_shell::init())
        .setup(|app| {
            let show = MenuItem::with_id(app, "show", "Open Wallpaper Engine", true, None::<&str>)?;
            let stop = MenuItem::with_id(app, "stop_all", "Stop All Wallpapers", true, None::<&str>)?;
            let sep  = PredefinedMenuItem::separator(app)?;
            let quit = MenuItem::with_id(app, "quit", "Quit", true, None::<&str>)?;
            let menu = Menu::with_items(app, &[&show, &stop, &sep, &quit])?;

            TrayIconBuilder::new()
                .icon(app.default_window_icon().unwrap().clone())
                .menu(&menu)
                .show_menu_on_left_click(false)
                .on_menu_event(|app, event| match event.id.as_ref() {
                    "quit" => app.exit(0),
                    "show" => {
                        if let Some(w) = app.get_webview_window("main") {
                            let _ = w.show();
                            let _ = w.set_focus();
                        }
                    }
                    "stop_all" => kill_all_wallpapers(),
                    _ => {}
                })
                .on_tray_icon_event(|tray, event| {
                    if let TrayIconEvent::Click {
                        button: MouseButton::Left,
                        button_state: MouseButtonState::Up,
                        ..
                    } = event
                    {
                        let app = tray.app_handle();
                        if let Some(w) = app.get_webview_window("main") {
                            let _ = w.show();
                            let _ = w.set_focus();
                        }
                    }
                })
                .build(app)?;

            // Close button hides to tray instead of quitting
            if let Some(window) = app.get_webview_window("main") {
                let w = window.clone();
                window.on_window_event(move |event| {
                    if let tauri::WindowEvent::CloseRequested { api, .. } = event {
                        let _ = w.hide();
                        api.prevent_close();
                    }
                });
            }

            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            get_displays,
            search_workshop,
            get_workshop_item,
            set_wallpaper,
            stop_wallpaper,
            get_wallpaper_status,
            check_steamcmd,
            check_tools,
            download_workshop_item,
            open_steamcmd_login,
            download_we_assets,
            scan_library,
            load_settings,
            save_settings,
            needs_onboarding,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
