// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
    id: page
    signal browseWorkshop()

    function refilter() {
        const types = ["all", "video", "scene", "web", "image"]
        App.filterLibrary(searchField.text, types[typeBar.currentIndex])
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Choose an image or video")
        nameFilters: [qsTr("Wallpapers (*.png *.jpg *.jpeg *.webp *.gif *.mp4 *.webm *.mkv *.mov)"), qsTr("All files (*)")]
        onAccepted: App.applyPath(selectedFile.toString())
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Pane {
            Layout.fillWidth: true
            padding: 12
            background: Rectangle { color: palette.window; Rectangle { y: parent.height - 1; width: parent.width; height: 1; color: Qt.alpha(palette.text, 0.12) } }
            RowLayout {
                anchors.fill: parent
                spacing: 12
                TextField { id: searchField; Layout.preferredWidth: 280; placeholderText: qsTr("Filter your library…"); onTextChanged: page.refilter() }
                TabBar {
                    id: typeBar
                    background: null
                    Repeater {
                        model: [qsTr("All"), qsTr("Video"), qsTr("Scene"), qsTr("Web"), qsTr("Image")]
                        TabButton { text: modelData; width: implicitWidth }
                    }
                    onCurrentIndexChanged: page.refilter()
                }
                Item { Layout.fillWidth: true }
                Button { text: qsTr("Use a file…"); icon.name: "document-open"; onClicked: fileDialog.open() }
                ToolButton { icon.name: "view-refresh"; text: "⟳"; onClicked: App.refreshLibrary(); ToolTip.visible: hovered; ToolTip.text: qsTr("Rescan") }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            GridView {
                id: grid
                anchors.fill: parent
                anchors.margins: 10
                clip: true
                model: App.library
                readonly property int columns: Math.max(2, Math.floor(width / 250))
                cellWidth: Math.floor(width / columns)
                cellHeight: Math.round(cellWidth * 0.78)
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {}

                delegate: Item {
                    width: grid.cellWidth; height: grid.cellHeight
                    WallpaperCard {
                        anchors.fill: parent; anchors.margins: 7
                        title: model.title
                        preview: model.previewUrl
                        kind: model.contentType
                        caption: model.workshopId !== "" ? qsTr("Workshop · %1").arg(model.workshopId) : qsTr("Local file")
                        active: model.active
                        badgeText: model.active ? qsTr("Now playing") : ""
                        hoverHint: qsTr("Click to apply")
                        onClicked: App.applyLibraryItem(index)
                    }
                }
            }

            ColumnLayout {
                anchors.centerIn: parent
                width: Math.min(parent.width - 80, 420)
                spacing: 12
                visible: grid.count === 0
                Label { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: qsTr("Nothing here yet"); font.pixelSize: 18; font.bold: true; color: palette.text }
                Label {
                    Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; wrapMode: Text.Wrap; opacity: 0.65; color: palette.text
                    text: qsTr("Download wallpapers from the Workshop, or point at an image or video on disk. Anything you've subscribed to in Steam shows up here too.")
                }
                Button { Layout.alignment: Qt.AlignHCenter; text: qsTr("Browse the Workshop"); highlighted: true; onClicked: page.browseWorkshop() }
            }
        }
    }
}
