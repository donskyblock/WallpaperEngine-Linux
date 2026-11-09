// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ScrollView {
    id: page
    signal openLogin()
    contentWidth: availableWidth
    clip: true

    readonly property bool onGnome: App.desktopId === "GNOME" || App.desktopId === "Budgie" || App.desktopId === "Pantheon"
    readonly property bool onPlasma: App.desktopId === "KDE Plasma"

    function load() {
        const s = App.settings
        keyField.text = s.steamApiKey
        dirField.text = s.downloadDir
        audioSwitch.checked = s.audioEnabled
        volumeSlider.value = s.volume
        fpsSpin.value = s.fpsLimit
        restoreSwitch.checked = s.restoreOnStart
        placementBox.currentIndex = Math.max(0, placementBox.model.findIndex(m => m.value === s.placementOverride))
    }
    Component.onCompleted: load()

    FolderDialog {
        id: folderDialog
        title: qsTr("Choose a download folder")
        onAccepted: dirField.text = selectedFolder.toString().replace(/^file:\/\//, "")
    }

    ColumnLayout {
        width: Math.min(page.availableWidth - 48, 760)
        x: Math.max(24, (page.availableWidth - width) / 2)
        y: 20
        spacing: 16

        Label { text: qsTr("Settings"); font.pixelSize: 24; font.bold: true; color: palette.text }

        Section {
            title: qsTr("Your desktop")
            subtitle: qsTr("How wallpapers get placed on this session. Detected automatically.")
            Label { text: App.environmentSummary; font.bold: true; color: palette.text; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Repeater { model: App.caveats; Label { text: "• " + modelData; wrapMode: Text.Wrap; Layout.fillWidth: true; opacity: 0.75; font.pixelSize: 12; color: palette.text } }
            RowLayout {
                Label { text: qsTr("Placement"); color: palette.text }
                ComboBox {
                    id: placementBox
                    textRole: "text"
                    model: [{text: qsTr("Automatic"), value: ""}, {text: "Wayland layer-shell", value: "layer-shell"}, {text: "X11 desktop window", value: "x11-desktop"},
                            {text: "GNOME Shell extension", value: "gnome-extension"}, {text: "KWin script", value: "kwin-script"}]
                }
            }
        }

        Section {
            title: qsTr("Steam")
            subtitle: qsTr("Needed for browsing and downloading Workshop wallpapers.")
            Label { text: qsTr("Web API key"); color: palette.text }
            RowLayout {
                Layout.fillWidth: true
                TextField { id: keyField; Layout.fillWidth: true; echoMode: showKey.checked ? TextInput.Normal : TextInput.Password; placeholderText: qsTr("32-character key") }
                CheckBox { id: showKey; text: qsTr("Show") }
                Button { text: qsTr("Get a key"); onClicked: App.openExternal("https://steamcommunity.com/dev/apikey") }
            }
            RowLayout {
                Layout.fillWidth: true
                Label { Layout.fillWidth: true; color: palette.text; text: App.settings.steamUsername !== "" ? qsTr("Account: %1").arg(App.settings.steamUsername) : qsTr("No account signed in") }
                Button { text: qsTr("Sign in…"); onClicked: page.openLogin() }
            }
        }

        Section {
            title: qsTr("Library")
            Label { text: qsTr("Download folder"); color: palette.text }
            RowLayout {
                Layout.fillWidth: true
                TextField { id: dirField; Layout.fillWidth: true }
                Button { text: qsTr("Browse…"); onClicked: folderDialog.open() }
            }
        }

        Section {
            title: qsTr("Playback")
            RowLayout {
                Label { text: qsTr("Audio"); color: palette.text; Layout.preferredWidth: 120 }
                Switch { id: audioSwitch }
                Slider { id: volumeSlider; from: 0; to: 100; stepSize: 5; enabled: audioSwitch.checked; Layout.preferredWidth: 220 }
                Label { text: Math.round(volumeSlider.value) + "%"; opacity: 0.6; color: palette.text }
            }
            RowLayout {
                Label { text: qsTr("Scene FPS limit"); color: palette.text; Layout.preferredWidth: 120 }
                SpinBox { id: fpsSpin; from: 10; to: 240; stepSize: 5 }
            }
            RowLayout {
                Label { text: qsTr("Restore on login"); color: palette.text; Layout.preferredWidth: 120 }
                Switch { id: restoreSwitch }
            }
        }

        Section {
            title: qsTr("Components")
            subtitle: qsTr("Image and video wallpapers need nothing else. Scenes need the renderer.")
            StatusRow { name: "wpe-wallpaper-host"; ok: App.tools.host; okText: qsTr("Found — images and video"); missingText: qsTr("Not found next to the application.") }
            StatusRow { name: "linux-wallpaperengine"; ok: App.tools.lwe; okText: qsTr("Found — Scene and Web wallpapers"); missingText: qsTr("Not found. Install it from your distro or the AUR.") }
            StatusRow {
                name: qsTr("Wallpaper Engine assets"); ok: App.tools.lweAssets
                okText: qsTr("Found — scenes can render"); missingText: qsTr("Not found. Install Wallpaper Engine in Steam, or download just the assets.")
                actionText: App.downloads.assets !== undefined ? qsTr("Working…") : qsTr("Download"); onAction: App.installAssets()
            }
            StatusRow { name: "steamcmd"; ok: App.tools.steamcmd; okText: qsTr("Found — Workshop downloads"); missingText: qsTr("Not found. Install steamcmd from your distro or the AUR.") }
            StatusRow {
                visible: page.onGnome; name: qsTr("GNOME Shell extension"); ok: App.tools.gnomeExtInstalled
                okText: qsTr("Installed. If animated wallpapers don't show, log out and back in once."); missingText: qsTr("Required for animated wallpapers on GNOME.")
                actionText: qsTr("Install"); onAction: App.installGnomeExtension()
            }
            StatusRow {
                visible: page.onPlasma; name: qsTr("Plasma wallpaper plugin"); ok: App.tools.plasmaPlugin
                okText: qsTr("Installed — video plays natively in Plasma"); missingText: qsTr("Lets video wallpapers keep your desktop icons and widgets.")
                actionText: qsTr("Install"); onAction: App.installPlasmaPlugin()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button { text: qsTr("Revert"); onClicked: page.load() }
            Button {
                text: qsTr("Save"); highlighted: true
                onClicked: App.saveSettings({
                    steamApiKey: keyField.text, downloadDir: dirField.text, audioEnabled: audioSwitch.checked,
                    volume: Math.round(volumeSlider.value), fpsLimit: fpsSpin.value, restoreOnStart: restoreSwitch.checked,
                    placementOverride: placementBox.model[placementBox.currentIndex].value
                })
            }
        }
        Item { Layout.preferredHeight: 20 }
    }
}
