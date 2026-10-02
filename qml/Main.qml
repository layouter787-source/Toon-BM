import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ToonBM

ApplicationWindow {
    id: win
    visible: true
    width: 1280
    height: 800
    title: "Toon-BM"
    color: "#2b2b2e"

    property color brush: "#000000"
    property bool eraser: false
    // Evita o "project: project" do CanvasItem se referir a si mesmo.
    property QtObject appProject: project

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Ferramentas
        ColumnLayout {
            Layout.preferredWidth: 96
            Layout.fillHeight: true
            Layout.margins: 6
            spacing: 6

            Button { text: "Caneta"; checkable: true; checked: !win.eraser; Layout.fillWidth: true; onClicked: win.eraser = false }
            Button { text: "Borracha"; checkable: true; checked: win.eraser; Layout.fillWidth: true; onClicked: win.eraser = true }
            Button { text: "Desfazer"; Layout.fillWidth: true; onClicked: project.undo() }
            Button { text: "Limpar"; Layout.fillWidth: true; onClicked: project.clearFrame() }

            Repeater {
                model: ["#000000", "#e53935", "#1e88e5", "#43a047", "#fdd835", "#8e24aa"]
                delegate: Rectangle {
                    required property string modelData
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    radius: 6
                    color: modelData
                    border.width: win.brush == modelData ? 3 : 1
                    border.color: "white"
                    MouseArea { anchors.fill: parent; onClicked: { win.brush = parent.modelData; win.eraser = false } }
                }
            }

            Label { text: "Tamanho"; color: "white" }
            Slider { id: sizeSlider; Layout.fillWidth: true; from: 1; to: 60; value: 6 }

            Item { Layout.fillHeight: true }
            Button { text: "Script"; Layout.fillWidth: true; onClicked: scriptPopup.open() }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Palco
            CanvasItem {
                Layout.fillWidth: true
                Layout.fillHeight: true
                project: win.appProject
                brushColor: win.brush
                brushSize: sizeSlider.value
                eraser: win.eraser
                onionSkin: onionBox.checked
            }

            // Timeline
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 130
                color: "#1e1e20"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 6

                    RowLayout {
                        spacing: 6
                        Button { text: project.playing ? "Pausar" : "Play"; onClicked: project.togglePlay() }
                        Button { text: "+ Frame"; onClicked: project.addFrame() }
                        Button { text: "Duplicar"; onClicked: project.duplicateFrame() }
                        Button { text: "Remover"; onClicked: project.removeFrame() }
                        CheckBox { id: onionBox; text: "Onion"; checked: true }
                        Label { text: "FPS"; color: "white" }
                        SpinBox { from: 1; to: 60; value: project.fps; editable: true; onValueModified: project.fps = value }
                        Label { text: (project.currentFrame + 1) + " / " + project.frameCount; color: "white" }
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        orientation: ListView.Horizontal
                        spacing: 4
                        clip: true
                        model: project.frameCount
                        delegate: Rectangle {
                            required property int index
                            width: 56
                            height: ListView.view.height
                            radius: 4
                            color: index === project.currentFrame ? "#3d85f5" : "#444"
                            Label { anchors.centerIn: parent; text: parent.index + 1; color: "white" }
                            MouseArea { anchors.fill: parent; onClicked: project.currentFrame = parent.index }
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: scriptPopup
        modal: true
        anchors.centerIn: Overlay.overlay
        width: Math.min(win.width - 40, 640)
        height: Math.min(win.height - 40, 420)

        ColumnLayout {
            anchors.fill: parent
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                TextArea {
                    id: scriptInput
                    placeholderText: "project.addFrame();\nproject.fps = 8;\nconsole.log(project.frameCount);"
                    wrapMode: TextArea.Wrap
                }
            }
            Button { text: "Executar"; onClicked: scriptOutput.text = scripts.run(scriptInput.text) }
            Label { id: scriptOutput; Layout.fillWidth: true; wrapMode: Text.Wrap }
        }
    }
}
