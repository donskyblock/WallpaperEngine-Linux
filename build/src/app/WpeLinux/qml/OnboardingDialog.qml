// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dlg
    modal: true
    closePolicy: Popup.NoAutoClose
    anchors.centerIn: Overlay.overlay
    width: 640
    height: 540
    padding: 0
    property int step: 0
    signal openLogin()

    background: Rectangle { radius: 12; color: dlg.palette.window; border.color: Qt.alpha(dlg.palette.text, 0.18) }

    function finish() { App.onboarding = false; close() }

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true; Layout.margins: 20; Layout.bottomMargin: 0; spacing: 6
            Repeater {
                model: 4
                Rectangle { height: 6; radius: 3; Layout.fillWidth: true; color: index <= dlg.step ? dlg.palette.highlight : Qt.alpha(dlg.palette.text, 0.15) }
            }
        }

        StackLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 28
            currentIndex: dlg.step

            // 0 — welcome + what we detected
            ColumnLayout {
                spacing: 12
                Label { text: qsTr("Welcome to Wallpaper Engine for Linux"); font.pixelSize: 24; font.bold: true; color: dlg.palette.text; wrapMode: Text.Wrap; Layout.fillWidth: true }
                Label { text: qsTr("We detected your session:"); opacity: 0.7; color: dlg.palette.text }
                Rectangle {
                    Layout.fillWidth: true; implicitHeight: det.implicitHeight + 24; radius: 8; color: Qt.alpha(dlg.palette.highlight, 0.12)
                    ColumnLayout {
                        id: det; anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 14 } spacing: 6
                        Label { text: App.environmentSummary; font.bold: true; color: dlg.palette.text; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Repeater { model: App.caveats; Label { text: "• " + modelData; wrapMode: Text.Wrap; Layout.fillWidth: true; opacity: 0.8; font.pixelSize: 12; color: dlg.palette.text } }
                    }
                }
                StatusRow { name: "linux-wallpaperengine"; ok: App.tools.lwe; okText: qsTr("Installed — renders Scene and Web wallpapers"); missingText: qsTr("Not found. Video and image wallpapers still work without it.") }
                StatusRow { name: "steamcmd"; ok: App.tools.steamcmd; okText: qsTr("Installed — Workshop downloads"); missingText: qsTr("Not found. Install it to download from the Workshop.") }
                Item { Layout.fillHeight: true }
            }

            // 1 — API key
            ColumnLayout {
                spacing: 12
                Label { text: qsTr("Steam Web API key"); font.pixelSize: 22; font.bold: true; color: dlg.palette.text }
                Label { Layout.fillWidth: true; wrapMode: Text.Wrap; opacity: 0.75; color: dlg.palette.text; text: qsTr("Needed to browse the Workshop. Create one for free (any domain name works), then paste it here.") }
                Button { text: qsTr("Open the Steam key page"); onClicked: App.openExternal("https://steamcommunity.com/dev/apikey") }
                TextField { id: keyField; Layout.fillWidth: true; placeholderText: qsTr("32-character key"); echoMode: TextInput.Password; text: App.settings.steamApiKey }
                Item { Layout.fillHeight: true }
            }

            // 2 — Steam login
            ColumnLayout {
                spacing: 12
                Label { text: qsTr("Steam account (optional)"); font.pixelSize: 22; font.bold: true; color: dlg.palette.text }
                Label { Layout.fillWidth: true; wrapMode: Text.Wrap; opacity: 0.75; color: dlg.palette.text; text: qsTr("Downloading Workshop wallpapers needs an account that owns Wallpaper Engine. You can do this later from Settings.") }
                Button { text: App.settings.steamUsername !== "" ? qsTr("Signed in as %1 — change").arg(App.settings.steamUsername) : qsTr("Sign in…"); onClicked: dlg.openLogin() }
                Item { Layout.fillHeight: true }
            }

            // 3 — done
            ColumnLayout {
                spacing: 12
                Item { Layout.fillHeight: true }
                Label { text: qsTr("You're set"); font.pixelSize: 26; font.bold: true; color: dlg.palette.text; Layout.alignment: Qt.AlignHCenter }
                Label {
                    Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.Wrap; opacity: 0.75; color: dlg.palette.text
                    text: qsTr("Closing the window keeps wallpapers running from the tray. Bind `wpectl random` or `wpectl set <file>` to a key in your compositor if you like.")
                }
                Item { Layout.fillHeight: true }
            }
        }

        RowLayout {
            Layout.fillWidth: true; Layout.margins: 20; Layout.topMargin: 0
            Button { text: qsTr("Skip setup"); flat: true; onClicked: dlg.finish() }
            Item { Layout.fillWidth: true }
            Button { text: qsTr("Back"); visible: dlg.step > 0; onClicked: dlg.step-- }
            Button {
                highlighted: true
                text: dlg.step === 3 ? qsTr("Get started") : qsTr("Next")
                onClicked: {
                    if (dlg.step === 1 && keyField.text.trim() !== "") App.saveSettings({steamApiKey: keyField.text})
                    if (dlg.step === 3) dlg.finish(); else dlg.step++
                }
            }
        }
    }
}
