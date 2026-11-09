// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls

Control {
    id: card
    property string title
    property url preview
    property string kind
    property string caption
    property bool active: false
    property bool badgeOk: false
    property string badgeText: ""
    property bool selected: false
    property string hoverHint: ""
    signal clicked()

    hoverEnabled: true
    scale: hover.hovered ? 1.015 : 1.0
    Behavior on scale { NumberAnimation { duration: 110; easing.type: Easing.OutCubic } }
    HoverHandler { id: hover; cursorShape: Qt.PointingHandCursor }
    TapHandler { onTapped: card.clicked() }

    background: Rectangle {
        radius: 10
        color: card.palette.base
        border.width: card.active || card.selected ? 2 : 1
        border.color: card.active || card.selected ? card.palette.highlight : Qt.alpha(card.palette.text, hover.hovered ? 0.30 : 0.12)
    }

    contentItem: Item {
        clip: true
        Rectangle { id: mask; anchors.fill: parent; radius: 9; visible: false }

        Rectangle {
            id: thumbFrame
            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 1 }
            height: parent.height - captionRow.height - 8
            radius: 9
            color: Qt.alpha(card.palette.text, 0.08)
            clip: true

            Image {
                anchors.fill: parent
                source: card.preview
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                cache: true
                smooth: true
                sourceSize.width: 520
                opacity: status === Image.Ready ? 1 : 0
                Behavior on opacity { NumberAnimation { duration: 180 } }
            }
            Label {
                anchors.centerIn: parent
                visible: card.preview == ""
                text: "🖼"
                font.pixelSize: 32
                opacity: 0.35
            }

            TypeBadge { anchors { left: parent.left; top: parent.top; margins: 8 } kind: card.kind }

            Rectangle {
                visible: card.badgeText !== ""
                anchors { right: parent.right; top: parent.top; margins: 8 }
                height: 20; width: badgeLabel.implicitWidth + 14; radius: 10
                color: card.badgeOk ? "#16a34a" : card.palette.highlight
                Label { id: badgeLabel; anchors.centerIn: parent; text: card.badgeText; color: "white"; font.pixelSize: 10; font.bold: true }
            }

            Rectangle {
                anchors.fill: parent
                visible: card.hoverHint !== "" && hover.hovered
                color: Qt.rgba(0, 0, 0, 0.42)
                Label { anchors.centerIn: parent; text: card.hoverHint; color: "white"; font.bold: true; font.pixelSize: 14 }
            }
        }

        Column {
            id: captionRow
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 10 }
            topPadding: 2
            spacing: 1
            Label { width: parent.width; text: card.title; elide: Text.ElideRight; font.bold: true; color: card.palette.text }
            Label { width: parent.width; text: card.caption; elide: Text.ElideRight; font.pixelSize: 11; opacity: 0.6; color: card.palette.text; visible: text !== "" }
        }
    }
}
