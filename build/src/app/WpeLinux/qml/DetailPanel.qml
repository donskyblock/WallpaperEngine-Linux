// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Right-hand details for one Workshop item (a map from WorkshopModel.get()).
Pane {
    id: root
    property var item: ({})
    signal closeRequested()

    readonly property string progress: item.itemId !== undefined && App.downloads[item.itemId] !== undefined ? App.downloads[item.itemId] : ""
    readonly property real rating: (item.votesUp + item.votesDown) > 0 ? item.votesUp / (item.votesUp + item.votesDown) : 0

    padding: 0
    background: Rectangle { color: root.palette.window; Rectangle { width: 1; height: parent.height; color: Qt.alpha(root.palette.text, 0.12) } }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: root.width
            spacing: 14

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: width * 0.62
                Image {
                    anchors.fill: parent
                    source: root.item.previewUrl ?? ""
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                }
                TypeBadge { anchors { left: parent.left; top: parent.top; margins: 12 } kind: root.item.contentType ?? "" }
                ToolButton {
                    anchors { right: parent.right; top: parent.top; margins: 6 }
                    icon.name: "window-close"
                    text: "✕"
                    onClicked: root.closeRequested()
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 18; Layout.rightMargin: 18
                spacing: 12

                Label { text: root.item.title ?? ""; font.bold: true; font.pixelSize: 20; wrapMode: Text.Wrap; Layout.fillWidth: true; color: root.palette.text }

                GridLayout {
                    columns: 2; columnSpacing: 20; rowSpacing: 8; Layout.fillWidth: true
                    Repeater {
                        model: [
                            {k: qsTr("Rating"), v: root.rating > 0 ? Math.round(root.rating * 100) + "%  (" + App.formatCount(root.item.votesUp + root.item.votesDown) + ")" : "–"},
                            {k: qsTr("Subscribers"), v: App.formatCount(root.item.subscriptions ?? 0)},
                            {k: qsTr("Views"), v: App.formatCount(root.item.views ?? 0)},
                            {k: qsTr("Size"), v: App.formatBytes(root.item.fileSize ?? 0)}
                        ]
                        delegate: ColumnLayout {
                            spacing: 0
                            Label { text: modelData.k; font.pixelSize: 11; opacity: 0.55; color: root.palette.text }
                            Label { text: modelData.v; font.bold: true; color: root.palette.text }
                        }
                    }
                }

                Flow {
                    Layout.fillWidth: true
                    spacing: 6
                    Repeater {
                        model: root.item.tags ?? []
                        delegate: Rectangle {
                            height: 22; width: tagLabel.implicitWidth + 16; radius: 11
                            color: Qt.alpha(root.palette.text, 0.09)
                            Label { id: tagLabel; anchors.centerIn: parent; text: modelData; font.pixelSize: 11; color: root.palette.text }
                        }
                    }
                }

                Label {
                    visible: text !== ""
                    text: root.item.description ?? ""
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    opacity: 0.8
                    color: root.palette.text
                }

                // ---- actions ----
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Button {
                        Layout.fillWidth: true
                        highlighted: true
                        visible: root.item.downloaded === true
                        text: qsTr("Apply wallpaper")
                        icon.name: "media-playback-start"
                        enabled: !App.applying
                        onClicked: App.applyWorkshopId(root.item.itemId)
                    }
                    Button {
                        Layout.fillWidth: true
                        highlighted: true
                        visible: root.item.downloaded !== true
                        enabled: root.progress === ""
                        text: root.progress === "" ? qsTr("Download") : qsTr("Downloading…")
                        icon.name: "folder-download"
                        onClicked: App.download(root.item.itemId)
                    }
                    Label {
                        visible: root.progress !== ""
                        text: root.progress
                        elide: Text.ElideMiddle
                        font.pixelSize: 11
                        opacity: 0.6
                        Layout.fillWidth: true
                        color: root.palette.text
                    }
                    ProgressBar { visible: root.progress !== ""; indeterminate: true; Layout.fillWidth: true }
                    Button {
                        Layout.fillWidth: true
                        flat: true
                        text: qsTr("Open on Steam")
                        icon.name: "internet-services"
                        onClicked: App.openExternal("https://steamcommunity.com/sharedfiles/filedetails/?id=" + root.item.itemId)
                    }
                }
                Item { Layout.preferredHeight: 12 }
            }
        }
    }
}
