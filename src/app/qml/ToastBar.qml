// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls

Control {
    id: root
    property string message
    property string kind: "info"

    function show(msg, k) { message = msg; kind = k; opacity = 1; hide.restart() }

    opacity: 0
    visible: opacity > 0
    Behavior on opacity { NumberAnimation { duration: 160 } }
    Timer { id: hide; interval: 4200; onTriggered: root.opacity = 0 }

    padding: 12; leftPadding: 16; rightPadding: 16
    width: Math.min(implicitWidth, 640)
    background: Rectangle {
        radius: 10
        color: root.kind === "error" ? "#b91c1c" : root.kind === "success" ? "#15803d" : root.palette.toolTipBase
        border.color: Qt.rgba(0, 0, 0, 0.2)
    }
    contentItem: Label {
        text: root.message
        wrapMode: Text.Wrap
        color: root.kind === "info" ? root.palette.toolTipText : "white"
    }
}
