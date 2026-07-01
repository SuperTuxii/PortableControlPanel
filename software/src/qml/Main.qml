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
}