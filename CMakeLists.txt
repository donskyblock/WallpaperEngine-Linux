# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.21)
project(wpe-linux VERSION 0.2.0 DESCRIPTION "Wallpaper Engine for Linux (native Qt frontend)" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# one bin/ dir so the app finds wpe-wallpaper-host next to itself in a build tree, same as when installed
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Network DBus Qml Quick QuickControls2 Multimedia Test)
qt_standard_project_setup()

# Wayland layer-shell placement (Hyprland, Niri, Sway, mango, River, labwc, Wayfire, COSMIC, ...)
# Provided by KDE's layer-shell-qt (Arch: layer-shell-qt, Fedora: layer-shell-qt-devel,
# Debian/Ubuntu: liblayershellqtinterface-dev). Must be the Qt6 build.
find_package(LayerShellQt QUIET)
# X11 desktop-window placement (XFCE, i3, bspwm, openbox, Cinnamon, MATE, LXQt, ...)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB QUIET IMPORTED_TARGET xcb)

message(STATUS "layer-shell-qt: ${LayerShellQt_FOUND}   xcb: ${XCB_FOUND}")

add_subdirectory(src/core)
add_subdirectory(src/host)
add_subdirectory(src/app)
add_subdirectory(src/cli)

enable_testing()
add_subdirectory(tests)

include(GNUInstallDirs)
install(FILES data/wpe-linux.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/wpe-linux.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps)
install(FILES data/wpe-linux.service DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/user)
install(FILES data/org.wpelinux.WallpaperEngine.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
