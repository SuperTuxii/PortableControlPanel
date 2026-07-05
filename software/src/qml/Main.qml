import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LvglSimulator

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("PortableControlPanel Software")
    color: Theme.mainBackground
    minimumWidth: mainLayout.implicitWidth
    minimumHeight: mainLayout.implicitHeight

    Settings {
        id: settings
        category: "Layouts"
        property string jsonData: "{}"
        property alias backlightBrightness: brightnessSlider.value

        function editLayout(name: string, data): void {
            if ("blocks" in data)
                return;
            let configData = JSON.parse(jsonData);
            if (!(name in configData))
                configData[name] = { rows: 3, columns: 5, blocks: [] };
            Object.assign(configData[name], data);
            jsonData = JSON.stringify(configData);
        }
        function removeLayout(name: string): void {
            let configData = JSON.parse(jsonData);
            if (name in configData) {
                delete configData[name];
                jsonData = JSON.stringify(configData);
            }
        }
        function saveBlock(layoutName: string, data: variant): void {
            let configData = JSON.parse(jsonData);
            if (!(layoutName in configData))
                configData[layoutName] = { rows: 3, columns: 5, blocks: [] };
            configData[layoutName].blocks.push(data);
            jsonData = JSON.stringify(configData);
        }
        function editBlock(layoutName: string, row: int, column: int, data): void {
            let configData = JSON.parse(jsonData);
            if (!(layoutName in configData))
                configData[layoutName] = { rows: 3, columns: 5, blocks: [] };
            for (let block of configData[layoutName].blocks) {
                if (row - block.row >= 0 && row - block.row < block.rowSpan
                    && column - block.column >= 0 && column - block.column < block.columnSpan) {
                    Object.assign(block, data);
                    jsonData = JSON.stringify(configData);
                    return;
                }
            }
            console.error("Tried editing Block at row", row, "and column", column, ", but wasn't found");
        }
        function removeBlock(layoutName: string, row: int, column: int): void {
            let configData = JSON.parse(jsonData);
            if (!(layoutName in configData))
                configData[layoutName] = { rows: 3, columns: 5, blocks: [] };
            for (let i in configData[layoutName].blocks) {
                let block = configData[layoutName].blocks[i];
                if (row - block.row >= 0 && row - block.row < block.rowSpan
                    && column - block.column >= 0 && column - block.column < block.columnSpan) {
                    configData[layoutName].blocks.splice(i, 1);
                    jsonData = JSON.stringify(configData);
                    return;
                }
            }
            console.error("Tried removing Block at row", row, "and column", column, ", but wasn't found");
        }
        function loadBlocks(layoutName: string, createCall: variant): void {
            let configData = JSON.parse(jsonData);
            if (!(layoutName in configData))
                configData[layoutName] = { rows: 3, columns: 5, blocks: [] };
            for (let block of configData[layoutName].blocks) {
                createCall(block);
            }
        }
        function loadBlock(layoutName: string, row: int, column: int): variant {
            let configData = JSON.parse(jsonData);
            if (!(layoutName in configData))
                configData[layoutName] = { rows: 3, columns: 5, blocks: [] };
            for (let block of configData[layoutName].blocks) {
                if (row - block.row >= 0 && row - block.row < block.rowSpan
                    && column - block.column >= 0 && column - block.column < block.columnSpan)
                    return block;
            }
            console.error("Tried loading Block at row", row, "and column", column, ", but wasn't found");
        }
        function loadLayout(name: string): variant {
            return JSON.parse(jsonData)[name];
        }
    }

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            color: Theme.secondaryBackground
            topRightRadius: Theme.sidebarRadius
            bottomRightRadius: Theme.sidebarRadius
            implicitWidth: Theme.sidebarWidth
            ColumnLayout {
                anchors.fill: parent
                spacing: 0
                Button {
                    implicitWidth: Theme.sidebarWidth - 2 * (Theme.sidebarRadius - Theme.sidebarButtonRadius)
                    implicitHeight: Theme.sidebarWidth - 2 * (Theme.sidebarRadius - Theme.sidebarButtonRadius)
                    Layout.margins: Theme.sidebarRadius - Theme.sidebarButtonRadius
                    text: Connection.connected ? Theme.icons.connection : Theme.icons.noConnection
                    font.family: Theme.iconFontName
                    font.weight: Theme.iconFontWeight
                    font.pixelSize: Theme.sidebarIconSize
                    background: Rectangle {
                        color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                        radius: Theme.sidebarButtonRadius
                        border.color: Connection.connected ? (parent.down ? Theme.buttonGreenActive : Theme.buttonGreen) : (parent.down ? Theme.buttonRedActive : Theme.buttonRed)
                        border.width: Theme.sidebarButtonBorderWidth
                    }
                    onClicked: Connection.tryConnect()
                }
                Slider {
                    id: brightnessSlider
                    implicitWidth: Theme.sidebarWidth - 2 * (Theme.sidebarRadius - Theme.sidebarButtonRadius)
                    implicitHeight: 100
                    from: 0
                    to: 1023
                    stepSize: 1
                    orientation: Qt.Vertical
                    Layout.margins: Theme.sidebarRadius - Theme.sidebarButtonRadius
                    handle: null
                    background: Rectangle {
                        radius: Theme.sidebarButtonRadius
                        border.color: Theme.border
                        border.width: Theme.sidebarButtonBorderWidth
                        anchors.fill: parent
                        gradient: Gradient {
                            orientation: Gradient.Vertical
                            GradientStop { position: 0.0; color: Theme.secondaryBorder }
                            GradientStop { position: Math.min(brightnessSlider.visualPosition, 0.99); color: Theme.secondaryBorder }
                            GradientStop { position: Math.min(brightnessSlider.visualPosition, 0.99) + 0.01; color: Theme.sliderActiveBackground }
                            GradientStop { position: 1.0; color: Theme.sliderActiveBackground }
                        }
                    }
                    Label {
                        anchors.fill: parent
                        anchors.bottomMargin: (parent.width - Theme.sidebarIconSize) / 2
                        text: parent.position > 0.66 ? Theme.icons.brightness3 :
                              parent.position > 0.33 ? Theme.icons.brightness2 : Theme.icons.brightness1
                        color: Qt.hsva(0, 0, parent.position, 1)
                        font.family: Theme.iconFontName
                        font.weight: Theme.iconFontWeight
                        font.pixelSize: Theme.sidebarIconSize
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignBottom
                    }
                    Timer {
                        id: brightnessDelayTimer
                        interval: 100
                        repeat: false
                        onTriggered: Connection.setBacklightBrightness(brightnessSlider.value);
                    }
                    onMoved: brightnessDelayTimer.start()
                }
                Rectangle {
                    implicitWidth: Theme.sidebarWidth - 2 * Theme.sidebarSeperatorMargin
                    implicitHeight: Theme.sidebarSeperatorWidth
                    color: Theme.border
                    radius: implicitHeight / 2
                    border.width: Theme.sidebarSeperatorBorderWidth
                    border.color: Theme.secondaryBackground
                    Layout.margins: Theme.sidebarSeperatorMargin
                }
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth
                    padding: Theme.sidebarRadius - Theme.sidebarButtonRadius
                    ColumnLayout {
                        id: sidebarLayout
                        spacing: 10
                        anchors.fill: parent
                    }
                }
                Rectangle {
                    implicitWidth: Theme.sidebarWidth - 2 * Theme.sidebarSeperatorMargin
                    implicitHeight: Theme.sidebarSeperatorWidth
                    color: Theme.border
                    radius: implicitHeight / 2
                    border.width: Theme.sidebarSeperatorBorderWidth
                    border.color: Theme.secondaryBackground
                    Layout.margins: Theme.sidebarSeperatorMargin
                }
                Button {
                    implicitWidth: Theme.sidebarWidth - 2 * (Theme.sidebarRadius - Theme.sidebarButtonRadius)
                    implicitHeight: Theme.sidebarWidth - 2 * (Theme.sidebarRadius - Theme.sidebarButtonRadius)
                    Layout.margins: Theme.sidebarRadius - Theme.sidebarButtonRadius
                    text: Theme.icons.add
                    font.family: Theme.iconFontName
                    font.weight: Theme.iconFontWeight
                    font.pixelSize: Theme.sidebarIconSize
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        opacity: parent.enabled ? 1.0 : 0.3
                        color: parent.down ? Theme.buttonGreenActive : Theme.buttonGreen
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                        radius: Theme.sidebarButtonRadius
                        border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                        border.width: Theme.sidebarButtonBorderWidth
                    }
                }
            }
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: displayFrame.width * displayFrame.displayScale
            Layout.minimumHeight: displayFrame.height * displayFrame.displayScale
            color: Theme.mainBackground

            Rectangle {
                id: displayFrame
                property real displayScale: 0.8

                anchors.centerIn: parent
                width: displayPanel.displayWidth + (2 * Theme.displayBorderRadius)
                height: displayPanel.displayHeight + (2 * Theme.displayBorderRadius)
                scale: displayScale
                radius: Theme.displayBorderRadius
                border.width: Theme.displayBorderRadius
                border.pixelAligned: true
                border.color: Theme.displayBorder
                color: Theme.displayBorder

                LvglDisplay {
                    id: displayPanel
                    anchors.centerIn: parent
                    Component.onDestruction: controlGrid.lvglRenderer = null
                }

                ControlGridQml {
                    id: controlGrid
                    displayPanel: displayPanel
                    settings: settings
                }

                ControlGridMouseArea {
                    controlGrid: controlGrid
                    controlGridBlockMenu: controlGridBlockMenu
                }
            }
        }
    }
    ControlGridBlockMenu {
        id: controlGridBlockMenu
        settings: settings
        controlGrid: controlGrid
        colorPicker: colorPicker
    }
    ColorPicker {
        id: colorPicker
    }
    Toastify {
        id: toastManager
    }

    Component.onCompleted: {
        Connection.connectedChanged.connect(onConnectionChanged);
        Connection.connectionError.connect(onConnectionError);
    }

    function onConnectionChanged(): void {
        if (controlGrid.rows === 0 && controlGrid.columns === 0) return;
        toastManager.createMessage("Serial Connection " + (Connection.connected ? "connected" : "disconnected"), {
            type: "info",
            position: "top-right",
            theme: "dark",
            autoClose: 1500,
            closeOnClick: true,
            hideProgressBar: false
        });
        if (!Connection.connected) return;
        Connection.setBacklightBrightness(brightnessSlider.value);
        Connection.setLayout(controlGrid.rows, controlGrid.columns);
        settings.loadBlocks(controlGrid.layoutName, (block) => {
            let index = (block.row * controlGrid.columns) + block.column;
            let index2 = index + (block.columnSpan-1) + ((block.rowSpan-1) * controlGrid.columns);
            Connection.addWidget(block.type, index, index2, block.style);
        });
    }

    function onConnectionError(error: string): void {
        toastManager.createMessage(error, {
            type: "error",
            position: "top-right",
            theme: "dark",
            autoClose: 3500,
            closeOnClick: true,
            hideProgressBar: false
        });
    }
}