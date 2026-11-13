// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: win
    width: 1280
    height: 800
    minimumWidth: 900
    minimumHeight: 560
    visible: App.windowVisible
    title: qsTr("Wallpaper Engine")
    property int currentPage: startPage

    // Closing hides to the tray; wallpapers keep running. "Quit" lives in the tray menu.
    onClosing: (close) => { close.accepted = false; App.windowVisible = false }

    Component.onCompleted: if (App.onboarding) onboarding.open()
    Connections { target: App; function onOnboardingChanged() { if (App.onboarding) onboarding.open() } }
    Connections { target: App; function onToast(message, kind) { toast.show(message, kind) } }

    readonly property var outputModel: [{name: "*", text: qsTr("All displays")}].concat(
        App.displays.map(d => ({name: d.name, text: d.name + "  —  " + d.width + "×" + d.height + (d.refresh > 0 ? " @ " + Math.round(d.refresh) + " Hz" : "")})))
    readonly property var activeHere: App.active.filter(a => App.selectedOutput === "*" || a.output === App.selectedOutput)

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ---- sidebar ----
        Pane {
            Layout.fillHeight: true
            Layout.preferredWidth: 212
            padding: 0
            background: Rectangle {
                color: Qt.tint(palette.window, Qt.rgba(0, 0, 0, 0.045))
                Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: Qt.alpha(palette.text, 0.12) }
            }
            ColumnLayout {
                anchors.fill: parent
                spacing: 2

                RowLayout {
                    Layout.margins: 16; Layout.bottomMargin: 14; spacing: 10
                    Image { source: "qrc:/icons/wpe-linux.png"; sourceSize: Qt.size(32, 32); Layout.preferredWidth: 32; Layout.preferredHeight: 32 }
                    ColumnLayout {
                        spacing: 0
                        Label { text: qsTr("Wallpaper Engine"); font.bold: true; color: palette.text }
                        Label { text: qsTr("for Linux"); font.pixelSize: 11; opacity: 0.55; color: palette.text }
                    }
                }

                Repeater {
                    model: [{t: qsTr("Workshop"), i: "internet-services", p: 0}, {t: qsTr("My Library"), i: "folder-pictures", p: 1}]
                    ItemDelegate {
                        Layout.fillWidth: true; Layout.leftMargin: 8; Layout.rightMargin: 8
                        text: modelData.t
                        icon.name: modelData.i
                        highlighted: win.currentPage === modelData.p
                        onClicked: win.currentPage = modelData.p
                    }
                }
                Item { Layout.fillHeight: true }
                ItemDelegate {
                    Layout.fillWidth: true; Layout.leftMargin: 8; Layout.rightMargin: 8
                    text: qsTr("Settings")
                    icon.name: "preferences-system"
                    highlighted: win.currentPage === 2
                    onClicked: win.currentPage = 2
                }
                Label {
                    Layout.fillWidth: true; Layout.margins: 14; Layout.topMargin: 6
                    text: App.environmentSummary
                    wrapMode: Text.Wrap; font.pixelSize: 11; opacity: 0.55; color: palette.text
                }
            }
        }

        // ---- pages ----
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: win.currentPage
            WorkshopPage { onOpenSettings: win.currentPage = 2 }
            LibraryPage { onBrowseWorkshop: win.currentPage = 0 }
            SettingsPage { onOpenLogin: login.open() }
        }
    }

    // ---- bottom bar: which display, what's playing, stop ----
    footer: ToolBar {
        background: Rectangle { color: palette.window; Rectangle { width: parent.width; height: 1; color: Qt.alpha(palette.text, 0.12) } }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14; anchors.rightMargin: 14
            spacing: 12
            Label { text: qsTr("Apply to"); opacity: 0.7; color: palette.text }
            ComboBox {
                Layout.preferredWidth: 300
                textRole: "text"
                model: win.outputModel
                currentIndex: Math.max(0, win.outputModel.findIndex(o => o.name === App.selectedOutput))
                onActivated: App.selectedOutput = win.outputModel[currentIndex].name
            }
            Label {
                Layout.fillWidth: true
                elide: Text.ElideMiddle
                color: palette.text
                opacity: 0.7
                text: win.activeHere.length === 0 ? qsTr("Nothing playing")
                      : win.activeHere.length === 1 ? qsTr("Playing on %1 · %2").arg(win.activeHere[0].output).arg(win.activeHere[0].path.split("/").slice(-2).join("/"))
                      : qsTr("Playing on %1 displays").arg(win.activeHere.length)
            }
            BusyIndicator { running: App.applying; visible: running; Layout.preferredHeight: 28; Layout.preferredWidth: 28 }
            Button { text: qsTr("Stop"); icon.name: "media-playback-stop"; enabled: win.activeHere.length > 0; onClicked: App.stopWallpaper() }
        }
    }

    SteamLoginDialog { id: login }
    OnboardingDialog { id: onboarding; onOpenLogin: login.open() }

    ToastBar { id: toast; anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 20 } z: 100 }
}
