// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property var selected: null
    property bool ready: false
    signal openSettings()

    readonly property bool needsKey: App.settings.steamApiKey === ""

    function reload() { App.search(searchField.text) }

    Component.onCompleted: { ready = true; if (!needsKey && App.workshop.rowCount() === 0) reload() }
    Connections { target: App; function onSettingsChanged() { if (!page.needsKey && App.workshop.rowCount() === 0) page.reload() } }

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
                TextField {
                    id: searchField
                    Layout.preferredWidth: 320
                    placeholderText: qsTr("Search the Steam Workshop…")
                    onAccepted: page.reload()
                }
                ComboBox {
                    id: sortBox
                    textRole: "text"; valueRole: "value"
                    model: [{text: qsTr("Most popular"), value: "vote"}, {text: qsTr("Newest"), value: "new"},
                            {text: qsTr("Trending"), value: "trend"}, {text: qsTr("Most subscribed"), value: "subscriptions"}]
                    onActivated: if (page.ready) App.setSort(currentValue)
                }
                TabBar {
                    id: typeBar
                    background: null
                    Repeater {
                        model: [{t: qsTr("All"), v: "all"}, {t: qsTr("Video"), v: "Video"}, {t: qsTr("Scene"), v: "Scene"}, {t: qsTr("Web"), v: "Web"}]
                        TabButton { text: modelData.t; width: implicitWidth; onClicked: if (page.ready) App.setTypeFilter(modelData.v) }
                    }
                }
                Item { Layout.fillWidth: true }
                Label {
                    visible: App.workshopTotal > 0
                    text: qsTr("%1 wallpapers").arg(App.formatCount(App.workshopTotal))
                    opacity: 0.6
                    color: palette.text
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                GridView {
                    id: grid
                    anchors.fill: parent
                    anchors.margins: 10
                    clip: true
                    model: App.workshop
                    readonly property int columns: Math.max(2, Math.floor(width / 250))
                    cellWidth: Math.floor(width / columns)
                    cellHeight: Math.round(cellWidth * 0.78)
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar {}
                    onAtYEndChanged: if (atYEnd && count > 0) App.loadMore()

                    delegate: Item {
                        width: grid.cellWidth; height: grid.cellHeight
                        WallpaperCard {
                            anchors.fill: parent; anchors.margins: 7
                            title: model.title
                            preview: model.previewUrl
                            kind: model.contentType
                            caption: qsTr("%1 subscribers").arg(App.formatCount(model.subscriptions))
                            selected: page.selected !== null && page.selected.itemId === model.itemId
                            badgeText: model.downloaded ? qsTr("✓ In library") : ""
                            badgeOk: true
                            onClicked: page.selected = App.workshop.get(index)
                        }
                    }

                    footer: Item {
                        width: grid.width; height: App.loadingMore ? 56 : 8
                        BusyIndicator { anchors.centerIn: parent; running: App.loadingMore; visible: running }
                    }
                }

                BusyIndicator { anchors.centerIn: parent; running: App.loadingWorkshop; visible: running; width: 56; height: 56 }

                ColumnLayout {
                    anchors.centerIn: parent
                    width: Math.min(parent.width - 80, 440)
                    spacing: 12
                    visible: !App.loadingWorkshop && grid.count === 0

                    Label {
                        Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter
                        text: page.needsKey ? qsTr("Add a Steam Web API key to browse the Workshop")
                                            : App.workshopError !== "" ? qsTr("Couldn't load the Workshop") : qsTr("No wallpapers found")
                        font.pixelSize: 18; font.bold: true; wrapMode: Text.Wrap; color: palette.text
                    }
                    Label {
                        Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter
                        visible: text !== ""
                        text: page.needsKey ? qsTr("A key is free and takes a minute. It only lets this app read public Workshop listings.") : App.workshopError
                        wrapMode: Text.Wrap; opacity: 0.65; color: palette.text
                    }
                    Button {
                        Layout.alignment: Qt.AlignHCenter
                        text: page.needsKey ? qsTr("Open Settings") : qsTr("Try again")
                        highlighted: true
                        onClicked: page.needsKey ? page.openSettings() : page.reload()
                    }
                }
            }

            DetailPanel {
                visible: page.selected !== null
                Layout.fillHeight: true
                Layout.preferredWidth: 380
                item: {
                    // keep the "downloaded" flag live while the panel is open
                    App.library.rowCount()
                    return page.selected ? page.selected : ({})
                }
                onCloseRequested: page.selected = null
            }
        }
    }
}
