import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ToonBM.Core 1.0

ApplicationWindow {
    id: win
    visible: true
    width: 1280
    height: 800
    title: "Toon-BM"
    color: "#2b2b2e"

    // Tela larga (tablet): camadas fixas na lateral. Tela estreita (celular): painel que abre pela borda.
    readonly property bool wide: width >= 900
    property color brush: "#000000"
    // 0 = caneta, 1 = borracha, 2 = selecionar, 3 = conta-gotas
    property int tool: 0
    // Evita o "project: project" do CanvasItem se referir a si mesmo.
    property QtObject appProject: project

    function askRename(layer) {
        renameField.text = project.layerName(layer)
        renamePopup.targetLayer = layer
        renamePopup.open()
    }

    // Aplica uma cor da paleta: recolore a seleção, ou volta para a caneta.
    function useColor(c) {
        win.brush = c
        if (win.tool === 2 && project.hasSelection) project.recolorSelected(c)
        else win.tool = 0
    }

    ListModel {
        id: paletteModel
        ListElement { swatch: "#000000" }
        ListElement { swatch: "#ffffff" }
        ListElement { swatch: "#e53935" }
        ListElement { swatch: "#fb8c00" }
        ListElement { swatch: "#fdd835" }
        ListElement { swatch: "#43a047" }
        ListElement { swatch: "#1e88e5" }
        ListElement { swatch: "#8e24aa" }
        ListElement { swatch: "#795548" }
    }

    // ---------- Painel de camadas (usado fixo e no Drawer) ----------
    component LayersPanel: ColumnLayout {
        id: panel
        signal renameRequested(int layer)
        spacing: 6

        Label { text: "Camadas"; color: "white"; font.bold: true }

        RowLayout {
            Layout.fillWidth: true
            spacing: 4
            Button { text: "+"; Layout.fillWidth: true; onClicked: project.addLayer() }
            Button { text: "−"; Layout.fillWidth: true; enabled: project.layerCount > 1; onClicked: project.removeLayer() }
            Button { text: "▲"; Layout.fillWidth: true; onClicked: project.moveLayerUp() }
            Button { text: "▼"; Layout.fillWidth: true; onClicked: project.moveLayerDown() }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 4
            model: project.layerCount
            delegate: Rectangle {
                id: row
                required property int index
                readonly property int li: project.layerCount - 1 - index
                width: ListView.view.width
                height: 48
                radius: 4
                color: li === project.currentLayer ? "#3d85f5" : "#444"

                MouseArea {
                    anchors.fill: parent
                    onClicked: project.currentLayer = row.li
                    onDoubleClicked: panel.renameRequested(row.li)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 8

                    // Olho: visível / oculta
                    Rectangle {
                        Layout.preferredWidth: 26
                        Layout.preferredHeight: 26
                        radius: 13
                        property bool shown: { project.layersRevision; return project.layerVisible(row.li) }
                        color: shown ? "white" : "#222"
                        border.color: "white"
                        MouseArea { anchors.fill: parent; onClicked: project.setLayerVisible(row.li, !parent.shown) }
                    }

                    Label {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        color: "white"
                        text: { project.layersRevision; return project.layerName(row.li) }
                    }
                }
            }
        }

        Label { text: "Opacidade"; color: "white" }
        Slider {
            id: opacitySlider
            Layout.fillWidth: true
            from: 0; to: 1
            value: 1
            onMoved: project.setLayerOpacity(project.currentLayer, value)
        }
        Connections {
            target: project
            function onCurrentLayerChanged() { opacitySlider.value = project.layerOpacity(project.currentLayer) }
        }
    }

    // ---------- Barra de ferramentas (topo) ----------
    header: ToolBar {
        contentHeight: 56
        background: Rectangle { color: "#1e1e20" }

        Flickable {
            anchors.fill: parent
            contentWidth: toolRow.width
            contentHeight: height
            flickableDirection: Flickable.HorizontalFlick
            clip: true

            Row {
                id: toolRow
                spacing: 6
                leftPadding: 6
                rightPadding: 6
                topPadding: 6

                Button { text: "Caneta"; highlighted: win.tool === 0; onClicked: win.tool = 0 }
                Button { text: "Borracha"; highlighted: win.tool === 1; onClicked: win.tool = 1 }
                Button { text: "Selecionar"; highlighted: win.tool === 2; onClicked: win.tool = 2 }
                Button { text: "Conta-gotas"; highlighted: win.tool === 3; onClicked: win.tool = 3 }
                Button { text: "Apagar seleção"; visible: project.hasSelection; onClicked: project.deleteSelected() }
                Button { text: "Desfazer"; onClicked: project.undo() }
                Button { text: "Limpar"; onClicked: project.clearFrame() }
                Button { text: "Ajustar"; onClicked: canvas.resetView() }
                Button { text: "Camadas"; visible: !win.wide; onClicked: layersDrawer.open() }
                Button { text: "Script"; onClicked: scriptPopup.open() }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Paleta de cores e tamanho do pincel
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 56
                color: "#232326"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 8

                    // Cor atual
                    Rectangle {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        radius: 6
                        color: win.brush
                        border.width: 2
                        border.color: "white"
                        MouseArea { anchors.fill: parent; onClicked: colorPopup.open() }
                    }

                    Flickable {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 44
                        contentWidth: swatchRow.width
                        contentHeight: height
                        flickableDirection: Flickable.HorizontalFlick
                        clip: true

                        Row {
                            id: swatchRow
                            spacing: 6
                            topPadding: 3

                            Repeater {
                                model: paletteModel
                                delegate: Rectangle {
                                    required property string swatch
                                    width: 38
                                    height: 38
                                    radius: 19
                                    color: swatch
                                    border.width: Qt.colorEqual(win.brush, swatch) ? 4 : 1
                                    border.color: "#3d85f5"
                                    MouseArea { anchors.fill: parent; onClicked: win.useColor(parent.swatch) }
                                }
                            }

                            Rectangle {
                                width: 38
                                height: 38
                                radius: 19
                                color: "#444"
                                border.color: "white"
                                Label { anchors.centerIn: parent; text: "+"; color: "white"; font.pixelSize: 22 }
                                MouseArea { anchors.fill: parent; onClicked: colorPopup.open() }
                            }
                        }
                    }

                    Slider { id: sizeSlider; Layout.preferredWidth: 130; from: 1; to: 60; value: 6 }
                }
            }

            // Palco (zoom e arrasto com dois dedos)
            CanvasItem {
                id: canvas
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 120
                project: win.appProject
                brushColor: win.brush
                brushSize: sizeSlider.value
                tool: win.tool
                onionSkin: onionBox.checked
                onColorPicked: (c) => { win.brush = c; win.tool = 0 }

                PinchHandler {
                    id: pinch
                    target: null
                    minimumPointCount: 2
                    maximumPointCount: 2
                    property real startZoom: 1
                    property real startPanX: 0
                    property real startPanY: 0
                    property real startCx: 0
                    property real startCy: 0
                    onActiveChanged: {
                        if (active) {
                            canvas.cancelStroke()
                            startZoom = canvas.zoom
                            startPanX = canvas.panX
                            startPanY = canvas.panY
                            startCx = centroid.position.x
                            startCy = centroid.position.y
                        }
                    }
                    onActiveScaleChanged: {
                        if (active) canvas.zoom = startZoom * activeScale
                    }
                    onCentroidChanged: {
                        if (active) {
                            canvas.panX = startPanX + centroid.position.x - startCx
                            canvas.panY = startPanY + centroid.position.y - startCy
                        }
                    }
                }
            }

            // Timeline
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 118
                color: "#1e1e20"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 6

                    Flickable {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 48
                        contentWidth: ctrlRow.width
                        contentHeight: height
                        flickableDirection: Flickable.HorizontalFlick
                        clip: true

                        Row {
                            id: ctrlRow
                            spacing: 6
                            Button { text: project.playing ? "Pausar" : "Play"; onClicked: project.togglePlay() }
                            Button { text: "+ Frame"; onClicked: project.addFrame() }
                            Button { text: "Duplicar"; onClicked: project.duplicateFrame() }
                            Button { text: "Remover"; onClicked: project.removeFrame() }
                            CheckBox { id: onionBox; text: "Onion"; checked: true }
                            Label { text: "FPS"; color: "white"; verticalAlignment: Text.AlignVCenter; height: 48 }
                            SpinBox { from: 1; to: 60; value: project.fps; editable: true; onValueModified: project.fps = value }
                            Label { text: (project.currentFrame + 1) + " / " + project.frameCount; color: "white"; verticalAlignment: Text.AlignVCenter; height: 48 }
                        }
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

        // Camadas fixas na lateral (só em tela larga)
        LayersPanel {
            visible: win.wide
            Layout.preferredWidth: 200
            Layout.fillHeight: true
            Layout.margins: 6
            onRenameRequested: (layer) => win.askRename(layer)
        }
    }

    // Camadas em painel deslizante (celular)
    Drawer {
        id: layersDrawer
        edge: Qt.RightEdge
        width: Math.min(win.width * 0.85, 320)
        height: win.height
        background: Rectangle { color: "#2b2b2e" }

        LayersPanel {
            anchors.fill: parent
            anchors.margins: 10
            onRenameRequested: (layer) => win.askRename(layer)
        }
    }

    // Seletor de cor (matiz, saturação e brilho)
    Popup {
        id: colorPopup
        modal: true
        anchors.centerIn: Overlay.overlay
        width: Math.min(win.width - 40, 340)
        property real h: 0
        property real s: 1
        property real v: 0
        readonly property color current: Qt.hsva(h, s, v, 1)

        onAboutToShow: {
            h = Math.max(0, win.brush.hsvHue)
            s = win.brush.hsvSaturation
            v = win.brush.hsvValue
            hueSlider.value = h
            satSlider.value = s
            valSlider.value = v
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 6

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                radius: 6
                color: colorPopup.current
                border.color: "#888"
            }

            Label { text: "Matiz" }
            Slider { id: hueSlider; Layout.fillWidth: true; from: 0; to: 1; onMoved: colorPopup.h = value }
            Label { text: "Saturação" }
            Slider { id: satSlider; Layout.fillWidth: true; from: 0; to: 1; onMoved: colorPopup.s = value }
            Label { text: "Brilho" }
            Slider { id: valSlider; Layout.fillWidth: true; from: 0; to: 1; onMoved: colorPopup.v = value }

            RowLayout {
                Layout.fillWidth: true
                Button {
                    text: "Usar"
                    Layout.fillWidth: true
                    onClicked: { win.useColor(colorPopup.current); colorPopup.close() }
                }
                Button {
                    text: "Salvar na paleta"
                    Layout.fillWidth: true
                    onClicked: {
                        paletteModel.append({ "swatch": colorPopup.current.toString() })
                        win.useColor(colorPopup.current)
                        colorPopup.close()
                    }
                }
            }
        }
    }

    Popup {
        id: renamePopup
        property int targetLayer: 0
        modal: true
        anchors.centerIn: Overlay.overlay
        width: Math.min(win.width - 40, 320)

        ColumnLayout {
            anchors.fill: parent
            Label { text: "Nome da camada" }
            TextField { id: renameField; Layout.fillWidth: true }
            Button {
                text: "OK"
                onClicked: { project.renameLayer(renamePopup.targetLayer, renameField.text); renamePopup.close() }
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
                    placeholderText: "project.addLayer();\nproject.addFrame();\nconsole.log(project.layerCount);"
                    wrapMode: TextArea.Wrap
                }
            }
            Button { text: "Executar"; onClicked: scriptOutput.text = scripts.run(scriptInput.text) }
            Label { id: scriptOutput; Layout.fillWidth: true; wrapMode: Text.Wrap }
        }
    }
}
