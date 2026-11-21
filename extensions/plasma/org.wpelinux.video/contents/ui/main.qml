// SPDX-License-Identifier: GPL-3.0-or-later
// Plasma 6 wallpaper plugin. Not yet run on a real Plasma session: please test and report.
import QtQuick
import QtMultimedia
import org.kde.plasma.plasmoid

WallpaperItem {
    id: root

    MediaPlayer {
        id: player
        source: root.configuration.VideoPath
        loops: MediaPlayer.Infinite
        videoOutput: output
        audioOutput: AudioOutput { muted: true }
        Component.onCompleted: play()
    }

    VideoOutput {
        id: output
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop
    }
}
