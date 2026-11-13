// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// A titled, rounded settings group.
Control {
    id: root
    property string title
    property string subtitle
    default property alias content: body.data

    Layout.fillWidth: true
    padding: 16
    background: Rectangle { radius: 10; color: root.palette.base; border.color: Qt.alpha(root.palette.text, 0.12) }
    contentItem: ColumnLayout {
        spacing: 10
        Label { text: root.title; font.bold: true; font.pixelSize: 15; color: root.palette.text }
        Label { visible: root.subtitle !== ""; text: root.subtitle; wrapMode: Text.Wrap; opacity: 0.65; Layout.fillWidth: true; color: root.palette.text }
        ColumnLayout { id: body; Layout.fillWidth: true; spacing: 10 }
    }
}
