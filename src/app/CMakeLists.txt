# SPDX-License-Identifier: GPL-3.0-or-later
qt_add_executable(wpe-linux main.cpp appcontroller.cpp models.cpp dbusapi.cpp)

qt_add_qml_module(wpe-linux
    URI WpeLinux
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_PLUGIN
    QML_FILES
        qml/Main.qml
        qml/WorkshopPage.qml
        qml/LibraryPage.qml
        qml/SettingsPage.qml
        qml/WallpaperCard.qml
        qml/DetailPanel.qml
        qml/TypeBadge.qml
        qml/SteamLoginDialog.qml
        qml/OnboardingDialog.qml
        qml/ToastBar.qml
        qml/StatusRow.qml
        qml/Section.qml
)

# GNOME Shell extension + Plasma wallpaper plugin ship inside the binary and get installed on demand.
qt_add_resources(wpe-linux ext_gnome PREFIX "/ext/gnome-shell"
    BASE ${CMAKE_SOURCE_DIR}/extensions/gnome-shell
    FILES ${CMAKE_SOURCE_DIR}/extensions/gnome-shell/metadata.json ${CMAKE_SOURCE_DIR}/extensions/gnome-shell/extension.js)
qt_add_resources(wpe-linux ext_plasma PREFIX "/ext/plasma"
    BASE ${CMAKE_SOURCE_DIR}/extensions/plasma/org.wpelinux.video
    FILES ${CMAKE_SOURCE_DIR}/extensions/plasma/org.wpelinux.video/metadata.json
          ${CMAKE_SOURCE_DIR}/extensions/plasma/org.wpelinux.video/contents/ui/main.qml
          ${CMAKE_SOURCE_DIR}/extensions/plasma/org.wpelinux.video/contents/config/main.xml)
qt_add_resources(wpe-linux icons PREFIX "/icons" BASE ${CMAKE_SOURCE_DIR}/data FILES ${CMAKE_SOURCE_DIR}/data/wpe-linux.png)

target_compile_definitions(wpe-linux PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(wpe-linux PRIVATE wpecore Qt6::Widgets Qt6::Quick Qt6::QuickControls2 Qt6::Qml Qt6::DBus Qt6::Network)
target_compile_options(wpe-linux PRIVATE -Wall -Wextra)
install(TARGETS wpe-linux DESTINATION ${CMAKE_INSTALL_BINDIR})
