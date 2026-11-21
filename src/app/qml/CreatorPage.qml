// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "qrc:/shared"

Item {
    id: page
    property var project: ({format: "wpe-native", version: 1, title: "My wallpaper", background: "#101820", layers: []})
    property int selected: -1
    property string savedPath: ""
    readonly property var layerData: selected >= 0 && selected < project.layers.length ? project.layers[selected] : ({})
    function update(key, value) {
        const copy = JSON.parse(JSON.stringify(project))
        if (selected >= 0) copy.layers[selected][key] = value
        project = copy
        savedPath = ""
    }
    function setCapability(name, enabled) {
        const copy = JSON.parse(JSON.stringify(project))
        copy.version = 2
        const caps = copy.capabilities || []
        copy.capabilities = caps.filter(c => c !== name)
        if (enabled) copy.capabilities.push(name)
        if (name === "dbus" && !enabled) delete copy.dbusSources
        project = copy; savedPath = ""
    }
    function editScript() {
        if (selected < 0) return
        scriptDialog.capabilities = (project.capabilities || []).slice()
        scriptMessage.text = ""
        scriptEditor.text = layerData.script || 'function(ctx) {\n    return {text: "Hello widget", rotation: Math.sin(ctx.time) * 5};\n}'
        dbusEditor.text = JSON.stringify(project.dbusSources || [], null, 2)
        scriptDialog.open()
    }
    function add(type, source) {
        const copy = JSON.parse(JSON.stringify(project))
        copy.layers.push({type: type, source: source || "", text: type === "clock" ? "hh:mm" : "Your text", color: "#ffffff", fontSize: 64,
                             x: 0, y: 0, width: 1, height: type === "text" || type === "clock" ? 0.2 : 1, opacity: 1})
        if (type === "widget") {
            copy.version = 2
            copy.layers[copy.layers.length - 1].height = 0.25
            copy.layers[copy.layers.length - 1].script = 'function(ctx) { return {text: ctx.media.title || "Nothing playing", background: "#243449", radius: 24}; }'
            copy.capabilities = Array.from(new Set((copy.capabilities || []).concat(["media"])))
        }
        project = copy; selected = copy.layers.length - 1; savedPath = ""
    }
    Dialog {
        id: scriptDialog
        property var capabilities: []
        function setCapability(name, enabled) {
            const caps = capabilities.filter(c => c !== name)
            if (enabled) caps.push(name)
            capabilities = caps
        }
        modal: true; anchors.centerIn: Overlay.overlay
        width: Math.min(900, page.width + 100); height: Math.min(700, page.height + 40)
        title: qsTr("JavaScript widget · function(ctx) → visual properties")
        standardButtons: Dialog.Cancel
        contentItem: ColumnLayout {
            Label { Layout.fillWidth: true; wrapMode: Text.Wrap; text: qsTr("ctx.time / delta, ctx.mouse, ctx.media and ctx.dbus feed your script. Return text, colours, transforms, and rect/text elements. No shell or file access.") }
            RowLayout {
                CheckBox { text: qsTr("Now playing"); checked: scriptDialog.capabilities.indexOf("media") >= 0; onClicked: scriptDialog.setCapability("media", checked) }
                CheckBox { text: qsTr("Pointer"); checked: scriptDialog.capabilities.indexOf("pointer") >= 0; onClicked: scriptDialog.setCapability("pointer", checked) }
                CheckBox { id: dbusCheck; text: qsTr("D-Bus properties"); checked: scriptDialog.capabilities.indexOf("dbus") >= 0; onClicked: scriptDialog.setCapability("dbus", checked) }
            }
            ScrollView {
                Layout.fillWidth: true; Layout.fillHeight: true
                TextArea { id: scriptEditor; objectName: "scriptEditor"; font.family: "monospace"; wrapMode: TextEdit.NoWrap; selectByMouse: true }
            }
            Label { visible: dbusCheck.checked; text: qsTr("Read-only sources JSON: [{id, bus, service, path, interface, property}]") }
            ScrollView {
                visible: dbusCheck.checked; Layout.fillWidth: true; Layout.preferredHeight: 110
                TextArea { id: dbusEditor; font.family: "monospace"; wrapMode: TextEdit.NoWrap; selectByMouse: true }
            }
            Label { id: scriptMessage; Layout.fillWidth: true; wrapMode: Text.Wrap; color: "#e76565" }
            RowLayout {
                Button {
                    text: qsTr("Mouse animation example")
                    onClicked: {
                        scriptDialog.setCapability("pointer", true)
                        scriptEditor.text = 'function(ctx) {\n  return {text: "Follow me", x: ctx.mouse.x * 0.7, y: ctx.mouse.y * 0.7, width: 0.3, height: 0.2, rotation: Math.sin(ctx.time * 2) * 10, background: "#315c78", radius: 24};\n}'
                    }
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Apply script"); highlighted: true
                    onClicked: {
                        try {
                            const sources = dbusCheck.checked ? JSON.parse(dbusEditor.text) : []
                            if (!Array.isArray(sources)) throw new Error("D-Bus sources must be an array")
                            const copy = JSON.parse(JSON.stringify(page.project)); copy.version = 2
                            copy.layers[page.selected].script = scriptEditor.text
                            copy.capabilities = scriptDialog.capabilities
                            if (!dbusCheck.checked) delete copy.dbusSources
                            if (dbusCheck.checked) copy.dbusSources = sources
                            page.project = copy; page.savedPath = ""; scriptMessage.text = ""; scriptDialog.close()
                        } catch (e) { scriptMessage.text = e.toString() }
                    }
                }
            }
        }
    }
    FileDialog {
        id: assetDialog
        property string kind: "image"
        title: qsTr("Choose a layer asset")
        nameFilters: kind === "image" ? ["Images (*.png *.jpg *.jpeg *.webp *.gif *.bmp *.avif)"] : ["Videos (*.mp4 *.webm *.mkv *.mov *.m4v *.avi)"]
        onAccepted: page.add(kind, selectedFile.toString())
    }
    FileDialog {
        id: openDialog
        title: qsTr("Open a native wallpaper")
        nameFilters: ["Native wallpaper (wallpaper.json)"]
        onAccepted: {
            const doc = App.studio.loadProject(selectedFile.toString())
            if (doc.format) { page.project = doc; page.selected = doc.layers.length ? 0 : -1; page.savedPath = "" }
        }
    }
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 16; spacing: 12
        RowLayout {
            Layout.fillWidth: true
            Label { text: qsTr("Wallpaper creator"); font.pixelSize: 22; font.bold: true }
            Item { Layout.fillWidth: true }
            Button { text: qsTr("New"); onClicked: { page.project = {format: "wpe-native", version: 1, title: "My wallpaper", background: "#101820", layers: []}; page.selected = -1; page.savedPath = "" } }
            Button { text: qsTr("Open…"); enabled: !App.studio.busy; onClicked: openDialog.open() }
            Button { text: qsTr("Save to library"); enabled: !App.studio.busy; highlighted: true; onClicked: page.savedPath = App.studio.saveProject(page.project) }
            Button { text: qsTr("Apply"); enabled: page.savedPath !== ""; onClicked: App.applyPath(page.savedPath) }
        }
        RowLayout {
            Label { text: qsTr("Title") }
            TextField { Layout.fillWidth: true; text: page.project.title; onTextEdited: { const p = Object.assign({}, page.project); p.title = text; page.project = p; page.savedPath = "" } }
            Label { text: qsTr("Background") }
            TextField { Layout.preferredWidth: 110; text: page.project.background; onTextEdited: { const p = Object.assign({}, page.project); p.background = text; page.project = p; page.savedPath = "" } }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 16
            ColumnLayout {
                Layout.preferredWidth: 205; Layout.fillHeight: true
                Label { text: qsTr("Layers · bottom to top"); font.bold: true }
                RowLayout {
                    Button { text: "+ Image"; onClicked: { assetDialog.kind = "image"; assetDialog.open() } }
                    Button { text: "+ Video"; onClicked: { assetDialog.kind = "video"; assetDialog.open() } }
                }
                RowLayout {
                    Button { text: "+ Text"; onClicked: page.add("text") }
                    Button { text: "+ Clock"; onClicked: page.add("clock") }
                }
                Button { text: qsTr("+ Scripted widget"); onClicked: { page.add("widget"); page.editScript() } }
                Button { text: qsTr("Edit layer script…"); enabled: page.selected >= 0; onClicked: page.editScript() }
                ListView {
                    Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                    model: page.project.layers
                    delegate: ItemDelegate { required property var modelData; required property int index; width: ListView.view.width; text: (index + 1) + " · " + modelData.type; highlighted: page.selected === index; onClicked: page.selected = index }
                }
                RowLayout {
                    Button {
                        text: qsTr("Raise"); enabled: page.selected >= 0 && page.selected < page.project.layers.length - 1
                        onClicked: { const p = JSON.parse(JSON.stringify(page.project)); const i = page.selected; const a = p.layers[i]; p.layers[i] = p.layers[i+1]; p.layers[i+1] = a; page.project = p; page.selected++; page.savedPath = "" }
                    }
                    Button {
                        text: qsTr("Remove"); enabled: page.selected >= 0
                        onClicked: { const p = JSON.parse(JSON.stringify(page.project)); p.layers.splice(page.selected, 1); page.project = p; page.selected = p.layers.length ? 0 : -1; page.savedPath = "" }
                    }
                }
            }
            ColumnLayout {
                Layout.fillWidth: true; Layout.fillHeight: true
                Item {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    NativeCanvas {
                        id: preview
                        objectName: "nativePreview"
                        anchors.centerIn: parent
                        width: Math.min(parent.width, parent.height * 16/9); height: width * 9/16
                        document: page.project; selectedLayer: page.selected
                        onLayerSelected: index => page.selected = index
                        onLayerMoved: (index, nx, ny) => { page.selected = index; page.update("x", nx); page.update("y", ny) }
                    }
                }
                Label { Layout.fillWidth: true; wrapMode: Text.Wrap; color: "#e76565"; visible: preview.scriptErrors.length > 0; text: preview.scriptErrors.join("\n") }
                Label { text: qsTr("Drag a layer to move it. Positions and sizes scale to each display."); wrapMode: Text.Wrap; Layout.fillWidth: true; opacity: 0.65 }
                GridLayout {
                    columns: 4; Layout.fillWidth: true; enabled: page.selected >= 0
                    Label { text: qsTr("Text / clock format") }
                    TextField { Layout.fillWidth: true; text: page.layerData.text || ""; onTextEdited: page.update("text", text) }
                    Label { text: qsTr("Text colour") }
                    TextField { text: page.layerData.color || "#ffffff"; onTextEdited: page.update("color", text) }
                    Label { text: qsTr("Font size") }
                    SpinBox { from: 1; to: 512; value: page.layerData.fontSize || 64; onValueModified: page.update("fontSize", value) }
                    Label { text: qsTr("Opacity %") }
                    SpinBox { from: 0; to: 100; value: Math.round((page.layerData.opacity === undefined ? 1 : page.layerData.opacity) * 100); onValueModified: page.update("opacity", value / 100) }
                    Repeater {
                        model: ["x", "y", "width", "height"]
                        RowLayout {
                            required property string modelData
                            Label { text: modelData + " %" }
                            SpinBox { from: modelData === "width" || modelData === "height" ? 1 : 0; to: 100; value: Math.round((page.layerData[modelData] || 0) * 100); onValueModified: page.update(modelData, value / 100) }
                        }
                    }
                }
            }
        }
        Label { visible: App.studio.error !== ""; text: App.studio.error; color: "#e76565"; wrapMode: Text.Wrap; Layout.fillWidth: true }
        RowLayout {
            visible: page.savedPath !== ""
            Label { text: qsTr("Saved. Share this folder through your own hosting or CDN."); Layout.fillWidth: true; wrapMode: Text.Wrap }
            Button { text: qsTr("Open project folder"); onClicked: Qt.openUrlExternally("file://" + page.savedPath) }
        }
    }
}
