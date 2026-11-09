// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls

Control {
    id: root
    property string kind: ""
    readonly property color tone: {
        switch (kind.toLowerCase()) {
        case "video": return "#3b82f6"
        case "scene": return "#8b5cf6"
        case "web": return "#f59e0b"
        case "image": return "#10b981"
        default: return "#6b7280"
        }
    }
    visible: kind !== "" && kind !== "unknown"
    leftPadding: 8; rightPadding: 8; topPadding: 2; bottomPadding: 2
    background: Rectangle { radius: height / 2; color: Qt.rgba(root.tone.r, root.tone.g, root.tone.b, 0.88) }
    contentItem: Label {
        text: root.kind.toUpperCase()
        color: "white"
        font.pixelSize: 10
        font.bold: true
        font.letterSpacing: 0.6
    }
}
