// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 20; spacing: 16
        Label { text: qsTr("Community wallpapers"); font.pixelSize: 24; font.bold: true }
        Label { text: qsTr("Browse an independent catalog. Wallpapers download directly from their creators’ hosting. Steam is optional."); wrapMode: Text.Wrap; Layout.fillWidth: true }
        RowLayout {
            Layout.fillWidth: true
            TextField { id: address; Layout.fillWidth: true; placeholderText: "https://…/catalog.json"; text: App.studio.catalogUrl; onAccepted: App.studio.refreshCatalog(text) }
            Button { text: qsTr("Load catalog"); enabled: !App.studio.busy && address.text !== ""; onClicked: App.studio.refreshCatalog(address.text) }
            BusyIndicator { running: App.studio.busy; visible: running; Layout.preferredWidth: 32; Layout.preferredHeight: 32 }
        }
        Label { visible: App.studio.error !== ""; text: App.studio.error; color: "#e76565"; Layout.fillWidth: true; wrapMode: Text.Wrap }
        Label { visible: App.studio.entries.length === 0 && !App.studio.busy; text: qsTr("Add a catalog URL above, import your own media in My Library, or build a wallpaper in Creator."); wrapMode: Text.Wrap; Layout.fillWidth: true; opacity: 0.7 }
        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true; clip: true; spacing: 8
            model: App.studio.entries
            ScrollBar.vertical: ScrollBar {}
            delegate: Pane {
                required property var modelData
                required property int index
                width: ListView.view.width
                contentItem: RowLayout {
                    ColumnLayout {
                        Layout.fillWidth: true
                        Label { text: modelData.title; font.bold: true; elide: Text.ElideRight; Layout.fillWidth: true }
                        Label { text: (modelData.author || "") + " · " + modelData.type + " · " + (modelData.license || "License not specified"); opacity: 0.65 }
                        Label { text: modelData.description || ""; wrapMode: Text.Wrap; Layout.fillWidth: true }
                    }
                    Button { text: qsTr("Add to library"); enabled: !App.studio.busy; onClicked: App.studio.install(index) }
                }
            }
        }
    }
    Component.onCompleted: if (App.studio.catalogUrl !== "" && App.studio.entries.length === 0) App.studio.refreshCatalog(App.studio.catalogUrl)
}
