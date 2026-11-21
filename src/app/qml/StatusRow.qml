// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    id: row
    property string name
    property bool ok: false
    property string okText
    property string missingText
    property string actionText
    signal action()
    spacing: 10
    Layout.fillWidth: true

    Rectangle { width: 10; height: 10; radius: 5; color: row.ok ? "#22c55e" : "#f59e0b"; Layout.alignment: Qt.AlignVCenter }
    ColumnLayout {
        spacing: 0
        Layout.fillWidth: true
        Label { text: row.name; font.bold: true; color: palette.text }
        Label { text: row.ok ? row.okText : row.missingText; wrapMode: Text.Wrap; Layout.fillWidth: true; opacity: 0.65; font.pixelSize: 12; color: palette.text }
    }
    Button { visible: row.actionText !== "" && !row.ok; text: row.actionText; onClicked: row.action() }
}
