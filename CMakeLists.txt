# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.21)
project(wpe-linux VERSION 0.2.0 DESCRIPTION "Wallpaper Engine for Linux (native Qt frontend)" LANGUAGES CXX)

include(GNUInstallDirs)
option(WPE_REQUIRE_LAYER_SHELL "Fail configuration if Qt 6 layer-shell support is unavailable" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# one bin/ dir so the app finds wpe-wallpaper-host next to itself in a build tree, same as when installed
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Network DBus Qml Quick QuickControls2 Multimedia Test)
qt_standard_project_setup()

# Wayland layer-shell placement (Hyprland, Niri, Sway, mango, River, labwc, Wayfire, COSMIC, ...)
find_package(LayerShellQt QUIET)
if(TARGET LayerShellQt::Interface)
    get_target_property(layer_shell_links LayerShellQt::Interface INTERFACE_LINK_LIBRARIES)
    if(layer_shell_links MATCHES "(^|;)Qt5::")
        message(FATAL_ERROR "LayerShellQt is linked to Qt 5. Install the Qt 6 build of layer-shell-qt.")
    endif()
elseif(WPE_REQUIRE_LAYER_SHELL)
    message(FATAL_ERROR
        "Qt 6 layer-shell-qt is required for Wayland wallpaper placement. Install layer-shell-qt "
        "(Arch), layer-shell-qt-devel (Fedora/Void), or liblayershellqtinterface-dev (Debian/Ubuntu). "
        "For a build that does not use Wayland layer-shell, configure with -DWPE_REQUIRE_LAYER_SHELL=OFF.")
endif()
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

install(FILES data/wpe-linux.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/wpe-linux.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps)
configure_file(data/wpe-linux.service wpe-linux.service @ONLY)
configure_file(data/org.wpelinux.WallpaperEngine.service org.wpelinux.WallpaperEngine.service @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/wpe-linux.service" DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/user)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.wpelinux.WallpaperEngine.service"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
