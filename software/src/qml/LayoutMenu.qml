import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls
import LvglSimulator

Popup {
    id: popup
    required property Settings settings
    required property ControlGridQml controlGrid
    required property BigTextBox bigTextBox
    required property ColorPicker colorPicker
    required property ImagesMenu imagesMenu
    required property SymbolListMenu symbolListMenu
    property string layoutName
    property var screenStyleData: {}

    anchors.centerIn: Overlay.overlay
    width: Math.max(Overlay.overlay.width / 2, Math.min(640, Overlay.overlay.width))
    height: Math.max(Overlay.overlay.height / 2, Math.min(640, Overlay.overlay.height))
    focus: true
    modal: true
    padding: 30

    background: Rectangle {
        color: Theme.secondaryBackground
        border.width: Theme.borderWidth
        border.color: Theme.border
        radius: Theme.borderRadius
    }

    RowLayout {
        uniformCellSizes: true
        anchors.fill: parent
        ColumnLayout {
            id: leftLayout
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                id: demoDisplay
                color: "#000000"
                topLeftRadius: 15
                topRightRadius: 15
                border.width: Theme.borderWidth
                border.color: Theme.secondaryBorder
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: childrenRect.height + 2 * Theme.borderWidth
                Layout.maximumHeight: childrenRect.height + 50

                LvglDisplay {
                    id: demoDisplayPanel
                    property real fitWidth: demoDisplay.width - 50
                    property real fitHeight: Math.max(150, (popup.height / 2.75) - 50)
                    name: "DemoLayoutDisplay"
                    implicitWidth: imageClipRect.width / imageClipRect.height * fitHeight < fitWidth ? imageClipRect.width / imageClipRect.height * fitHeight : fitWidth
                    implicitHeight: imageClipRect.width / imageClipRect.height * fitHeight > fitWidth ? imageClipRect.height / imageClipRect.width * fitWidth : fitHeight
                    anchors.centerIn: parent
                    Component.onCompleted: popup.controlGrid.displayPanel.displaySizeRefreshed.connect(() => {
                        demoDisplayPanel.changeDisplaySize(
                            popup.controlGrid.displayPanel.displayWidth,
                            popup.controlGrid.displayPanel.displayHeight
                        );
                        demoControlGrid.setLayout(1, 1);
                    })
                    Component.onDestruction: demoControlGrid.lvglRenderer = null
                }

                ControlGrid {
                    id: demoControlGrid
                    Component.onCompleted: {
                        demoDisplayPanel.transferRenderer(demoControlGrid);
                        setLayout(1, 1);
                    }
                }
            }
            StyleDataView {
                id: styleDataView
                bigTextBox: popup.bigTextBox
                colorPicker: popup.colorPicker
                imagesMenu: popup.imagesMenu
                onStyleDataUpdated: popup.updateDemoDisplayLive()
            }
        }
        ColumnLayout {
            id: rightLayout
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            Layout.fillHeight: true

            Label {
                text: "Configuration:"
                font.pointSize: 15
                font.underline: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                Layout.bottomMargin: 10
            }
            StringMenuValue {
                id: nameMenuValue
                attrKey: "name"
                propName: "Name"
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
            }
            StringMenuValue {
                id: displayMenuValue
                attrKey: "display"
                propName: "Display"
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
                onInputPressed: {
                    if (typeMenuValue.value === "Symbol" && !popup.symbolListMenu.visible)
                        openSymbolListMenu();
                }
                onValueChanged: {
                    if (typeMenuValue.value === "Symbol" && !popup.symbolListMenu.visible)
                        openSymbolListMenu();
                }
                function openSymbolListMenu(): void {
                    popup.symbolListMenu.x = mapToItem(null, 0, 0).x;
                    popup.symbolListMenu.y = mapToItem(null, 0, height).y;
                    popup.symbolListMenu.width = Qt.binding(() => width);
                    popup.symbolListMenu.height = Qt.binding(() => width);
                    popup.symbolListMenu.searchString = Qt.binding(() => value);
                    popup.symbolListMenu.selected.connect(onSymbolListMenuSelected);
                    popup.symbolListMenu.closed.connect(onSymbolListMenuClosed);
                    popup.symbolListMenu.open()
                }
                function onSymbolListMenuSelected(symbolName: string): void {
                    popup.symbolListMenu.searchString = "";
                    value = symbolName;
                    popup.symbolListMenu.close();
                }
                function onSymbolListMenuClosed(): void {
                    popup.symbolListMenu.selected.disconnect(onSymbolListMenuSelected);
                    popup.symbolListMenu.closed.disconnect(onSymbolListMenuClosed);
                }
            }
            OptionMenuValue {
                id: typeMenuValue
                attrKey: "displayType"
                propName: "Display Type"
                options: ["Text", "Symbol"]
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
            }
            IntMenuValue {
                id: rowsMenuValue
                attrKey: "rows"
                propName: "Rows"
                min: 1
                max: 256 / columnsMenuValue.numberValue
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
                onNumberValueChanged: {
                    if (!popup.visible || demoControlGrid.rows === numberValue) return;
                    demoControlGrid.setLayout(numberValue, columnsMenuValue.numberValue);
                    demoControlGrid.testFill();
                }
            }
            IntMenuValue {
                id: columnsMenuValue
                attrKey: "columns"
                propName: "Columns"
                min: 1
                max: 256 / rowsMenuValue.numberValue
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
                onNumberValueChanged: {
                    if (!popup.visible || demoControlGrid.columns === numberValue) return;
                    demoControlGrid.setLayout(rowsMenuValue.numberValue, numberValue);
                    demoControlGrid.testFill();
                }
            }
            IntMenuValue {
                id: outerPadMenuValue
                attrKey: "outerPad"
                propName: "Outer Pad"
                min: 0
                max: Math.max(popup.controlGrid.displayPanel.displayWidth, popup.controlGrid.displayPanel.displayHeight) / 2
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
                onNumberValueChanged: {
                    if (!popup.visible || demoControlGrid.outerPad === numberValue) return;
                    demoControlGrid.outerPad = numberValue;
                }
            }
            IntMenuValue {
                id: rowPadMenuValue
                attrKey: "rowPad"
                propName: "Row Pad"
                min: 0
                max: (popup.controlGrid.displayPanel.displayWidth - (outerPadMenuValue.numberValue * 2)) / rowsMenuValue.numberValue
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
                onNumberValueChanged: {
                    if (!popup.visible || demoControlGrid.rowPad === numberValue) return;
                    demoControlGrid.rowPad = numberValue;
                }
            }
            IntMenuValue {
                id: columnPadMenuValue
                attrKey: "columnPad"
                propName: "Column Pad"
                min: 0
                max: (popup.controlGrid.displayPanel.displayHeight - (outerPadMenuValue.numberValue * 2)) / columnsMenuValue.numberValue
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
                onNumberValueChanged: {
                    if (!popup.visible || demoControlGrid.columnPad === numberValue) return;
                    demoControlGrid.columnPad = numberValue;
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: removeButton.implicitWidth + 10 + saveButton.implicitWidth
                Layout.minimumHeight: Math.max(removeButton.implicitHeight, saveButton.implicitHeight)
                Button {
                    id: removeButton
                    implicitHeight: 30
                    horizontalPadding: 10
                    text: Theme.icons.trash
                    font.family: Theme.iconFontName
                    font.weight: Theme.iconFontWeight
                    font.pixelSize: Theme.iconFontSize
                    anchors.bottom: parent.bottom
                    anchors.right: saveButton.left
                    anchors.rightMargin: 10
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        opacity: parent.enabled ? 1.0 : 0.3
                        color: parent.down ? Theme.buttonRedActive : Theme.buttonRed
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                        radius: Theme.buttonRadius
                        border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                        border.width: Theme.buttonBorderWidth
                    }

                    onClicked: deleteDialog.open()
                }
                Button {
                    id: saveButton
                    implicitHeight: 30
                    horizontalPadding: 10
                    text: Theme.icons.save
                    font.family: Theme.iconFontName
                    font.weight: Theme.iconFontWeight
                    font.pixelSize: Theme.iconFontSize
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
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
                        radius: Theme.buttonRadius
                        border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                        border.width: Theme.buttonBorderWidth
                    }

                    onClicked: popup.saveLayout()
                }
            }
            Label {
                id: failLabel
                visible: false
                color: Theme.labelRed
                font.pointSize: 8
                Layout.alignment: Qt.AlignRight
            }
        }
    }

    MessageDialog {
        id: deleteDialog
        text: "Delete Layout?"
        informativeText: "Do you really want to delete this layout?"
        buttons: MessageDialog.Ok | MessageDialog.Cancel

        onAccepted: popup.removeLayout()
    }

    onOpened: {
        failLabel.text = "";
        failLabel.visible = false;
        colorPicker.x = Qt.binding(() => Math.min(Overlay.overlay.width - colorPicker.width, popup.x + popup.width));
        colorPicker.y = Qt.binding(() => popup.y + (popup.height - colorPicker.height) / 2);
        demoControlGrid.clearImages();
        demoControlGrid.removeScreenStyles();
        loadLayout();
        symbolListMenu.close();
    }

    onClosed: {
        demoControlGrid.clear();
        demoControlGrid.removeScreenStyles();
        symbolListMenu.close();
    }

    onXChanged: {
        if (typeMenuValue.value !== "Symbol" || !symbolListMenu.visible) return;
        symbolListMenu.x = displayMenuValue.mapToItem(null, 0, 0).x;
    }
    onYChanged: {
        if (typeMenuValue.value !== "Symbol" || !symbolListMenu.visible) return;
        symbolListMenu.y = displayMenuValue.mapToItem(null, 0, displayMenuValue.height).y;
    }

    function loadLayout(): void {
        const layoutData = settings.loadLayout(layoutName);
        demoControlGrid.setLayout(layoutData.rows, layoutData.columns);
        demoControlGrid.outerPad = layoutData.outerPad;
        demoControlGrid.rowPad = layoutData.rowPad;
        demoControlGrid.columnPad = layoutData.columnPad;
        screenStyleData = layoutData.style ?? {};
        buildMacros();
        styleDataView.styleData = Qt.binding(() => screenStyleData);
        for (let menuValue of rightLayout.children) {
            if (menuValue.attrKey === undefined || !menuValue.visible ||  menuValue.attrKey === "name") continue;
            if (layoutData[menuValue.attrKey] === undefined) {
                if (menuValue instanceof StringMenuValue)
                    menuValue.value = "";
                else if (menuValue instanceof IntMenuValue)
                    menuValue.value = menuValue.min > 0 ? menuValue.min : 0;
                continue;
            }
            menuValue.value = layoutData[menuValue.attrKey];
        }
        nameMenuValue.value = layoutName;
        demoControlGrid.testFill();
    }

    function saveLayout(): void {
        if (!nameMenuValue.value) {
            failLabel.text = "A name is required for a layout"
            failLabel.visible = true;
            return;
        }
        if (nameMenuValue.value !== layoutName && settings.layoutExists(nameMenuValue.value)) {
            failLabel.text = "A layout with this name already exists"
            failLabel.visible = true;
            return;
        }
        styleDataView.updateStyleData();
        const data = { style: styleDataView.styleData };
        for (let menuValue of rightLayout.children) {
            if (menuValue.attrKey === undefined || !menuValue.visible || menuValue.attrKey === "name") continue;
            if (menuValue instanceof IntMenuValue)
                data[menuValue.attrKey] = menuValue.numberValue;
            else
                data[menuValue.attrKey] = menuValue.value;
        }
        if (layoutName && nameMenuValue.value !== layoutName) {
            settings.renameLayout(layoutName, nameMenuValue.value);
            settings.editLayout(nameMenuValue.value, data);
            if (controlGrid.layoutName === layoutName)
                controlGrid.layoutName = nameMenuValue.value;
        } else {
            layoutName = nameMenuValue.value;
            settings.editLayout(layoutName, data);
            if (controlGrid.layoutName === layoutName)
                controlGrid.loadLayout();
        }
        popup.close();
    }

    function removeLayout(): void {
        if (controlGrid.layoutName === layoutName)
            controlGrid.layoutName = "";
        settings.removeLayout(layoutName);
        popup.close();
    }

    function updateDemoDisplayLive(): void {
        if (!popup.visible) return;
        demoControlGrid.removeScreenStyles();

        buildMacros();
        styleDataView.revalidateStyleValue();
        updateImagesLive();

        for (const styleSelector in screenStyleData) {
            demoControlGrid.setScreenStyle(styleSelector, screenStyleData[styleSelector]);
        }
    }

    function updateImagesLive(): void {
        let images = new Set();
        for (const styleSelector in screenStyleData) {
            for (const styleElement of screenStyleData[styleSelector]) {
                if (styleElement.attrKey === Connection.BackgroundImageIndex
                    && styleElement.value && styleElement.value.imageKey && styleElement.value.imageKey.length > 1)
                    images.add(styleElement.value.imageKey);
            }
        }
        demoControlGrid.loadImages(Array.from(images));
    }

    function buildMacros(): void {
        const mainMacros = {};
        Utils.buildScreenMacros(
            mainMacros,
            screenStyleData,
            demoDisplayPanel.displayWidth,
            demoDisplayPanel.displayHeight,
            demoControlGrid.rows,
            demoControlGrid.columns,
            demoControlGrid.outerPad,
            demoControlGrid.rowPad,
            demoControlGrid.columnPad
        );
        styleDataView.macros = mainMacros;
    }
}