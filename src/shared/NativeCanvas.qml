// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Window
import QtMultimedia
import Wpe.Native 1.0

Rectangle {
    id: canvas
    property var document: ({})
    property url baseUrl
    property int volume: 0
    property int selectedLayer: -1
    property var externalPointer: ({})
    readonly property bool runtimeActive: visible && (!Window.window || (Window.window.visible && Window.window.visibility !== Window.Minimized))
    readonly property var scriptErrors: widgets.errors
    readonly property bool pointerEnabled: document.version === 2 && (document.capabilities || []).indexOf("pointer") >= 0
    signal layerSelected(int index)
    signal layerMoved(int index, real nx, real ny)
    signal playbackFailed(string message)
    color: document.background || "#101820"
    clip: true
    property date now: new Date()
    Timer { interval: 1000; running: canvas.runtimeActive; repeat: true; onTriggered: canvas.now = new Date() }
    WidgetRuntime {
        id: widgets
        objectName: "widgetRuntime"
        document: canvas.document
        active: canvas.runtimeActive
        surfaceWidth: canvas.width
        surfaceHeight: canvas.height
        onErrorsChanged: { for (const message of errors) console.log("Widget:", message) }
    }
    onExternalPointerChanged: if (externalPointer.available) widgets.setPointer(externalPointer.x, externalPointer.y, externalPointer.inside, externalPointer.pressed)
    // Observes local motion without claiming mouse presses; editor dragging remains available.
    HoverHandler {
        enabled: canvas.pointerEnabled && !canvas.externalPointer.available
        onPointChanged: widgets.setPointer(point.position.x / Math.max(1, canvas.width), point.position.y / Math.max(1, canvas.height), hovered, false)
        onHoveredChanged: widgets.setPointer(point.position.x / Math.max(1, canvas.width), point.position.y / Math.max(1, canvas.height), hovered, false)
    }
    TapHandler {
        enabled: canvas.pointerEnabled && !canvas.externalPointer.available
        acceptedButtons: Qt.LeftButton
        onPressedChanged: widgets.setPointer(point.position.x / Math.max(1, canvas.width), point.position.y / Math.max(1, canvas.height), true, pressed)
    }
    function asset(path) {
        if (!path) return ""
        return /^(file:|qrc:)/.test(path) ? path : baseUrl.toString() + path.split("/").map(encodeURIComponent).join("/")
    }
    Repeater {
        model: canvas.document.layers || []
        delegate: Item {
            id: nativeLayer
            required property var modelData
            required property int index
            readonly property var frame: widgets.frames[index] || ({})
            function field(key, fallback) { return frame[key] !== undefined ? frame[key] : modelData[key] !== undefined ? modelData[key] : fallback }
            x: field("x", 0) * canvas.width
            y: field("y", 0) * canvas.height
            width: field("width", 1) * canvas.width
            height: field("height", 1) * canvas.height
            opacity: field("opacity", 1)
            rotation: field("rotation", 0)
            scale: field("scale", 1)
            clip: true
            Loader {
                anchors.fill: parent
                sourceComponent: nativeLayer.modelData.type === "video" ? video : nativeLayer.modelData.type === "image" ? image : nativeLayer.modelData.type === "widget" ? widget : text
            }
            Component {
                id: image
                Item {
                    readonly property url imageSource: canvas.asset(nativeLayer.modelData.source)
                    Image {
                        anchors.fill: parent
                        visible: !/\.(gif|webp|apng)$/i.test(parent.imageSource.toString())
                        source: visible ? parent.imageSource : ""
                        fillMode: Image.PreserveAspectCrop
                        onStatusChanged: if (status === Image.Error) canvas.playbackFailed("Cannot load image: " + source)
                    }
                    AnimatedImage {
                        anchors.fill: parent
                        visible: /\.(gif|webp|apng)$/i.test(parent.imageSource.toString())
                        source: visible ? parent.imageSource : ""
                        fillMode: Image.PreserveAspectCrop
                        playing: canvas.runtimeActive
                        onStatusChanged: if (status === Image.Error) canvas.playbackFailed("Cannot load image: " + source)
                    }
                }
            }
            Component {
                id: video
                Item {
                    MediaPlayer {
                        id: player
                        source: canvas.asset(nativeLayer.modelData.source)
                        loops: MediaPlayer.Infinite
                        videoOutput: output
                        audioOutput: AudioOutput { volume: canvas.volume / 100; muted: canvas.volume === 0 }
                        Component.onCompleted: if (canvas.runtimeActive) play()
                        onErrorOccurred: (error, message) => canvas.playbackFailed(message)
                    }
                    Connections {
                        target: canvas
                        function onRuntimeActiveChanged() { if (canvas.runtimeActive) player.play(); else player.pause() }
                    }
                    VideoOutput { id: output; anchors.fill: parent; fillMode: VideoOutput.PreserveAspectCrop }
                }
            }
            Component {
                id: text
                Text {
                    text: nativeLayer.modelData.type === "clock" && nativeLayer.frame.text === undefined ? Qt.formatDateTime(canvas.now, nativeLayer.modelData.text || "hh:mm") : nativeLayer.field("text", "")
                    textFormat: Text.PlainText
                    color: nativeLayer.field("color", "white")
                    font.pixelSize: nativeLayer.field("fontSize", 64) * canvas.height / 1080
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                }
            }
            Component {
                id: widget
                Rectangle {
                    color: nativeLayer.field("background", "transparent")
                    radius: nativeLayer.field("radius", 0) * canvas.height / 1080
                    Text {
                        anchors.fill: parent
                        text: nativeLayer.field("text", "")
                        textFormat: Text.PlainText
                        color: nativeLayer.field("color", "white")
                        font.pixelSize: nativeLayer.field("fontSize", 48) * canvas.height / 1080
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; wrapMode: Text.Wrap
                    }
                    Repeater {
                        model: nativeLayer.frame.elements || []
                        delegate: Rectangle {
                            id: primitive
                            required property var modelData
                            x: (modelData.x || 0) * nativeLayer.width
                            y: (modelData.y || 0) * nativeLayer.height
                            width: (modelData.width === undefined ? 1 : modelData.width) * nativeLayer.width
                            height: (modelData.height === undefined ? 1 : modelData.height) * nativeLayer.height
                            opacity: modelData.opacity === undefined ? 1 : modelData.opacity
                            rotation: modelData.rotation || 0
                            color: modelData.type === "text" ? (modelData.background || "transparent") : (modelData.background || modelData.color || "white")
                            radius: (modelData.radius || 0) * canvas.height / 1080
                            Text {
                                anchors.fill: parent
                                visible: primitive.modelData.type === "text"
                                text: primitive.modelData.text || ""
                                textFormat: Text.PlainText
                                color: primitive.modelData.color || "white"
                                font.pixelSize: (primitive.modelData.fontSize || 48) * canvas.height / 1080
                                horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; wrapMode: Text.Wrap
                            }
                        }
                    }
                }
            }
            Rectangle { anchors.fill: parent; color: "transparent"; border.color: "#67d9ed"; border.width: 2; visible: canvas.selectedLayer === nativeLayer.index }
            MouseArea {
                anchors.fill: parent
                enabled: canvas.selectedLayer >= 0
                property real startX
                property real startY
                onPressed: mouse => { canvas.layerSelected(nativeLayer.index); startX = mouse.x; startY = mouse.y }
                onReleased: mouse => canvas.layerMoved(nativeLayer.index, Math.max(0, Math.min(1, (nativeLayer.x + mouse.x - startX) / canvas.width)), Math.max(0, Math.min(1, (nativeLayer.y + mouse.y - startY) / canvas.height)))
            }
        }
    }
}
