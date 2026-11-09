// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Window
import QtMultimedia

Rectangle {
    id: root
    property string kind: "image"
    property url source
    property int volume: 0
    color: "black"

    readonly property bool animatedImage: /\.(gif|webp|apng)$/i.test(source.toString())

    Loader {
        anchors.fill: parent
        sourceComponent: root.kind === "video" ? videoComponent : (root.animatedImage ? animatedComponent : stillComponent)
    }

    Component {
        id: stillComponent
        Image {
            source: root.source
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            cache: false
            smooth: true
            sourceSize: Qt.size(root.width * Screen.devicePixelRatio, root.height * Screen.devicePixelRatio)
            onStatusChanged: if (status === Image.Error) { console.error("cannot load image:", source); Qt.exit(2) }
        }
    }

    Component {
        id: animatedComponent
        AnimatedImage {
            source: root.source
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            cache: false
            playing: true
            onStatusChanged: if (status === Image.Error) { console.error("cannot load image:", source); Qt.exit(2) }
        }
    }

    Component {
        id: videoComponent
        Item {
            MediaPlayer {
                id: player
                source: root.source
                loops: MediaPlayer.Infinite
                videoOutput: output
                audioOutput: AudioOutput { volume: root.volume / 100; muted: root.volume === 0 }
                onErrorOccurred: (error, message) => { console.error("playback error:", message); Qt.exit(3) }
                Component.onCompleted: play()
            }
            VideoOutput {
                id: output
                anchors.fill: parent
                fillMode: VideoOutput.PreserveAspectCrop
            }
        }
    }
}
