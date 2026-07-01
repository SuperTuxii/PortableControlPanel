import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls
import LvglSimulator

Popup {
    id: popup
    readonly property list<string> blockTypes: ["Button"]
    required property Settings settings
    required property ControlGridQml controlGrid
    required property ColorPicker colorPicker
    property bool newBlock
    property int row
    property int column
    property int rows: controlGrid.rows
    property int columns: controlGrid.columns

    anchors.centerIn: Overlay.overlay
    width: Math.max(Overlay.overlay.width / 2, Math.min(640, Overlay.overlay.width))
    height: Math.max(Overlay.overlay.height / 2, Math.min(640, Overlay.overlay.height))
    focus: true
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
                Layout.minimumWidth: childrenRect.width
                Layout.maximumHeight: childrenRect.height + 50

                LvglDisplay {
                    id: demoDisplayPanel
                    name: "DemoDisplay"
                    displayWidth: 130
                    displayHeight: 130
                    anchors.centerIn: parent
                    Component.onDestruction: demoControlGrid.lvglRenderer = null
                }

                ControlGrid {
                    id: demoControlGrid
                    Component.onCompleted: {
                        demoDisplayPanel.transferRenderer(demoControlGrid);
                        outerPad = 15;
                        setLayout(1, 1);
                    }
                }
            }
            ScrollView {
                id: menuValueScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                ScrollBar.vertical.interactive: false
                clip: true

                ColumnLayout {
                    id: menuValueLayout
                    property var styleData: {}
                    property int styleSelector: 0
                    Component.onCompleted: styleSelector = stateComboBox.currentValue | partComboBox.currentValue

                    // Style Selector ComboBoxes (State & Part)
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            Layout.preferredWidth: Math.floor((menuValueScroll.availableWidth - parent.spacing) / 2)
                            text: "State"
                            color: Theme.labelWhite
                            horizontalAlignment: Qt.AlignHCenter
                        }
                        Text {
                            Layout.preferredWidth: Math.floor((menuValueScroll.availableWidth - parent.spacing) / 2)
                            text: "Part"
                            color: Theme.labelWhite
                            horizontalAlignment: Qt.AlignHCenter
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        ComboBox {
                            id: stateComboBox
                            Layout.preferredWidth: Math.floor((menuValueScroll.availableWidth - parent.spacing) / 2)
                            background: Rectangle {
                                color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                                radius: Theme.buttonRadius
                                border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                                border.width: Theme.buttonBorderWidth
                            }
                            model: {
                                let values = [
                                    { text: Connection.styleStateString(Connection.StateDefault), value: Connection.StateDefault }
                                ];
                                for (let i = 1; i < Connection.StateAny; i<<=1) {
                                    let key = Connection.styleStateString(i);
                                    if (key) {
                                        values.push({text: key, value: i});
                                    }
                                }
                                values.push({ text: Connection.styleStateString(Connection.StateAny), value: Connection.StateAny });
                                return values;
                            }
                            textRole: "text"
                            valueRole: "value"

                            onActivated: {
                                if (menuValueLayout.styleSelector !== (stateComboBox.currentValue | partComboBox.currentValue))
                                    popup.changeStyleSelector();
                            }
                        }
                        ComboBox {
                            id: partComboBox
                            Layout.preferredWidth: Math.floor((menuValueScroll.availableWidth - parent.spacing) / 2)
                            background: Rectangle {
                                color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                                radius: Theme.buttonRadius
                                border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                                border.width: Theme.buttonBorderWidth
                            }
                            model: {
                                let values = [
                                    { text: Connection.stylePartString(Connection.PartMain), value: Connection.PartMain }
                                ];
                                for (let i = 1; i < Connection.PartAny; i<<=1) {
                                    let key = Connection.stylePartString(i);
                                    if (key) {
                                        values.push({ text: key, value: i });
                                    }
                                }
                                values.push({ text: Connection.stylePartString(Connection.PartAny), value: Connection.PartAny });
                                return values;
                            }
                            textRole: "text"
                            valueRole: "value"

                            onActivated: {
                                if (menuValueLayout.styleSelector !== (stateComboBox.currentValue | partComboBox.currentValue))
                                    popup.changeStyleSelector();
                            }
                        }
                    }

                    // Style Values (with Drag-Handling)
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: styleValueLayout.implicitHeight
                        ColumnLayout {
                            id: styleValueLayout
                            readonly property var data: {
                                let data = [];
                                for (let child of children) {
                                    if (child === styleValuePlaceholder) {
                                        if (!child.reference) continue;
                                        child = child.reference;
                                    }
                                    if (child instanceof DirectionsMenuValue)
                                        data.push({ attrKey: parseInt(child.attrKey), name: child.propName, value: Array.from(child.numberValues) });
                                    else if (child instanceof ColorMenuValue)
                                        data.push({ attrKey: parseInt(child.attrKey), name: child.propName, value: (child.value.a === 1 ? child.value.toString() + "FF" : "#" + child.value.toString().slice(3, 9) + child.value.toString().slice(1, 3)).toUpperCase() });
                                    else
                                        data.push({ attrKey: parseInt(child.attrKey), name: child.propName, value: child.value });
                                }
                                dirty = true;
                                Qt.callLater(() => {
                                    if (!styleValueLayout.dirty || !popup.visible) return;
                                    demoControlGrid.remove(0, 0);
                                    demoControlGrid.addWidget(typeMenuValue.value, 0, 0);
                                    demoControlGrid.setStyle(0, 0, menuValueLayout.styleSelector, styleValueLayout.data);
                                    for (const styleSelector in menuValueLayout.styleData) {
                                        if (parseInt(styleSelector) === menuValueLayout.styleSelector) continue;
                                        demoControlGrid.setStyle(0, 0, styleSelector, menuValueLayout.styleData[styleSelector]);
                                    }
                                    styleValueLayout.dirty = false;
                                });
                                return data;
                            }
                            property bool dirty: false
                            anchors.fill: parent
                        }
                        MouseArea {
                            id: styleValueLayoutArea
                            property real startX
                            property real startY
                            property real childY
                            width: styleValueLayout.width / 2
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom
                            cursorShape: Qt.DragMoveCursor
                            acceptedButtons: Qt.LeftButton
                            preventStealing: true

                            onPressed: (mouse) => {
                                if (styleValueLayoutArea.children.length > 0)
                                    styleValueLayoutArea.children[0].parent = styleValueLayout;
                                let index = Math.trunc(mouse.y / (30 + styleValueLayout.spacing));
                                let child = styleValueLayout.children[index];
                                if (!child) return;
                                startX = mouse.x;
                                startY = mouse.y - child.y;
                                childY = child.y;
                                child.parent = styleValueLayoutArea;
                                styleValuePlaceholder.parent = styleValueLayout;
                                styleValuePlaceholder.Layout.fillWidth = true;
                                styleValuePlaceholder.reference = child;
                                styleValuePlaceholder.visible = true;
                                while (styleValueLayout.children[index] !== styleValuePlaceholder) {
                                    let moveChild = styleValueLayout.children[index];
                                    moveChild.parent = null;
                                    moveChild.parent = styleValueLayout;
                                }
                            }
                            onPositionChanged: (mouse) => {
                                if (!pressed || styleValueLayoutArea.children.length <= 0) return;
                                let child = styleValueLayoutArea.children[0];
                                if (mouse.x < startX && Math.abs(mouse.x - startX) > Math.abs(mouse.y - startY - childY)) {
                                    child.x = Math.max(mouse.x - startX, -45);
                                    child.y = childY;
                                } else {
                                    child.x = 0;
                                    child.y = Math.min(Math.max(mouse.y - startY, 0), height - 30);
                                }
                                let index = Math.round(child.y / (30 + styleValueLayout.spacing));
                                if (styleValueLayout.children[index] !== styleValuePlaceholder) {
                                    styleValuePlaceholder.parent = null;
                                    styleValuePlaceholder.parent = styleValueLayout;
                                    while (styleValueLayout.children[index] !== styleValuePlaceholder) {
                                        let moveChild = styleValueLayout.children[index];
                                        moveChild.parent = null;
                                        moveChild.parent = styleValueLayout;
                                    }
                                }
                            }
                            onReleased: (mouse) => {
                                if (styleValueLayoutArea.children.length <= 0) return;
                                let child = styleValueLayoutArea.children[0];
                                let index = styleValueLayout.children.findIndex(item => item === styleValuePlaceholder);
                                styleValuePlaceholder.parent = parent;
                                styleValuePlaceholder.visible = false;
                                styleValuePlaceholder.reference = undefined;
                                if (mouse.x - startX <= -45 && Math.abs(mouse.x - startX) > Math.abs(mouse.y - startY - childY)) {
                                    child.destroy();
                                    child.parent = null;
                                } else {
                                    child.x = 0;
                                    child.y = 0;
                                    child.parent = styleValueLayout;
                                    while (styleValueLayout.children[index] !== child) {
                                        let moveChild = styleValueLayout.children[index];
                                        moveChild.parent = null;
                                        moveChild.parent = styleValueLayout;
                                    }
                                }
                            }
                        }
                        Item {
                            id: styleValuePlaceholder
                            property var reference
                            visible: false
                            implicitHeight: 30
                            Label {
                                x: parent.width + (styleValueLayoutArea.children.length > 0 ? styleValueLayoutArea.children[0].x + 15 : 0)
                                width: 30
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                text: Theme.icons.remove
                                font.family: Theme.iconFontName
                                font.weight: Theme.iconFontWeight
                                font.pixelSize: Theme.iconFontSize
                                horizontalAlignment: Text.AlignHCenter
                                background: Rectangle {
                                    color: Theme.buttonRed
                                    topLeftRadius: Theme.buttonRadius
                                    bottomLeftRadius: Theme.buttonRadius
                                }
                            }
                        }
                    }

                    StyleKeysAdder {
                        Layout.fillWidth: true
                    }
                }
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
            OptionMenuValue {
                id: typeMenuValue
                attrKey: "type"
                propName: "Type"
                options: popup.blockTypes
                Layout.maximumHeight: 30
            }
            IntMenuValue {
                id: rowMenuValue
                attrKey: "row"
                propName: "Row"
                min: 0
                max: popup.rows - 1
                Layout.maximumHeight: 30
            }
            IntMenuValue {
                id: columnMenuValue
                attrKey: "column"
                propName: "Column"
                min: 0
                max: popup.columns - 1
                Layout.maximumHeight: 30
            }
            IntMenuValue {
                id: rowSpanMenuValue
                attrKey: "rowSpan"
                propName: "Row Span"
                min: 1
                max: popup.rows - rowMenuValue.value
                Layout.maximumHeight: 30
            }
            IntMenuValue {
                id: columnSpanMenuValue
                attrKey: "columnSpan"
                propName: "Column Span"
                min: 1
                max: popup.columns - columnMenuValue.value
                Layout.maximumHeight: 30
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: removeButton.implicitWidth + 10 + saveButton.implicitWidth
                Layout.minimumHeight: removeButton.implicitHeight + saveButton.implicitHeight
                Button {
                    id: removeButton
                    implicitHeight: 30
                    horizontalPadding: 10
                    text: Theme.icons.trash
                    font.family: Theme.iconFontName
                    font.weight: Theme.iconFontWeight
                    font.pixelSize: Theme.iconFontSize
                    enabled: !popup.newBlock
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

                    onClicked: popup.saveBlock()
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
        text: "Delete Block?"
        informativeText: "Do you really want to delete this block?"
        buttons: MessageDialog.Ok | MessageDialog.Cancel

        onAccepted: popup.removeBlock()
    }

    onOpened: {
        failLabel.text = "";
        failLabel.visible = false;
        colorPicker.x = Qt.binding(() => Math.min(Overlay.overlay.width - colorPicker.width, popup.x + popup.width));
        colorPicker.y = Qt.binding(() => popup.y + (popup.height - colorPicker.height) / 2);
        let blockData = newBlock ? undefined : settings.loadBlock(controlGrid.layoutName, row, column);
        if (blockData !== undefined)
            typeMenuValue.value = blockData["type"];
        else
            typeMenuValue.value = popup.blockTypes[0];
        stateComboBox.currentIndex = 0;
        partComboBox.currentIndex = 0;
        clearStyleValues();
        loadBlock(blockData);
        loadDemoDisplay();
    }

    onClosed: {
        demoControlGrid.remove(0, 0);
    }

    function loadDemoDisplay(): void {
        demoControlGrid.remove(0, 0);
        demoControlGrid.addWidget(typeMenuValue.value, 0, 0);
        for (const styleSelector in menuValueLayout.styleData) {
            demoControlGrid.setStyle(0, 0, styleSelector, menuValueLayout.styleData[styleSelector]);
        }
    }
    function loadBlock(data): void {
        if (!newBlock) {
            menuValueLayout.styleData = data.style ?? {};
            menuValueLayout.styleSelector = -1;
            changeStyleSelector();
            for (let menuValue of rightLayout.children) {
                if (menuValue.attrKey === undefined) continue;
                menuValue.value = data[menuValue.attrKey];
            }
        } else {
            menuValueLayout.styleData = {};
            rowSpanMenuValue.value = 1;
            columnSpanMenuValue.value = 1;
            rowMenuValue.value = row;
            columnMenuValue.value = column;
        }
    }
    function saveBlock(): void {
        if (styleValueLayout.data.length !== 0)
            menuValueLayout.styleData[menuValueLayout.styleSelector] = styleValueLayout.data;
        else if (menuValueLayout.styleSelector in menuValueLayout.styleData)
            delete menuValueLayout.styleData[menuValueLayout.styleSelector];
        let data = {
            style: menuValueLayout.styleData,
        };
        for (let menuValue of rightLayout.children) {
            if (menuValue.attrKey === undefined) continue;
            data[menuValue.attrKey] = menuValue.value;
        }
        if (!newBlock) {
            controlGrid.remove((row * columns) + column, 0);
            Connection.remove((row * columns) + column, 0);
        }
        if (controlGrid.addBlock(data)) {
            if (newBlock) {
                settings.saveBlock(controlGrid.layoutName, data);
            } else {
                settings.editBlock(controlGrid.layoutName, row, column, data)
            }
            popup.close();
        } else {
            data = settings.loadBlock(controlGrid.layoutName, row, column);
            if (data !== undefined)
                controlGrid.addBlock(data);
            failLabel.text = "Can't place block here";
            failLabel.visible = true;
        }
    }
    function removeBlock(): void {
        if (newBlock)
            return;
        settings.removeBlock(controlGrid.layoutName, row, column);
        controlGrid.remove((row * controlGrid.columns) + column, 0);
        Connection.remove((row * controlGrid.columns) + column, 0);
        popup.close();
    }

    // Style Value Layout
    function addStyleValue(componentPath: string, data): variant {
        const component = Qt.createComponent(componentPath);
        if (component.status === Component.Error) {
            console.error(component.errorString());
        } else if (component.status === Component.Ready) {
            data["Layout.preferredHeight"] = 30;
            data["Layout.fillWidth"] = true;
            return component.createObject(styleValueLayout, data);
        } else {
            console.error("component not ready yet");
        }
    }
    function clearStyleValues(): void {
        while (styleValueLayout.children.length > 0) {
            styleValueLayout.children[0].destroy();
            styleValueLayout.children[0].parent = null;
        }
    }
    function addStyleKeyValue(styleKeyValue: int, styleKeyText: string): variant {
        if ((styleKeyValue >= Connection.NumberStyleKeyMin && styleKeyValue <= Connection.NumberStyleKeyMax)
            || (styleKeyValue >= Connection.Number16StyleKeyMin && styleKeyValue <= Connection.Number16StyleKeyMax)) {
            if (styleKeyValue === Connection.PadAll || styleKeyValue === Connection.MarginAll) {
                return addStyleValue("menuValues/DirectionsMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    directions: 4,
                    min: -(1 << (styleKeyValue <= Connection.NumberStyleKeyMax ? 31 : 15)),
                    max: (1 << (styleKeyValue <= Connection.NumberStyleKeyMax ? 31 : 15)) - 1,
                });
            } else if (Connection.styleKeyString(styleKeyValue).endsWith("X")
                || styleKeyValue === Connection.TransformWidth
                || styleKeyValue === Connection.TranslateScale
                || (styleKeyValue >= Connection.BackgroundGradParams1 && styleKeyValue <= Connection.BackgroundGradParams2)) {
                return addStyleValue("menuValues/DirectionsMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    directions: 2,
                    min: -(1 << (styleKeyValue <= Connection.NumberStyleKeyMax ? 31 : 15)),
                    max: (1 << (styleKeyValue <= Connection.NumberStyleKeyMax ? 31 : 15)) - 1,
                });
            } else {
                return addStyleValue("menuValues/IntMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    min: -(1 << (styleKeyValue <= Connection.NumberStyleKeyMax ? 31 : 15)),
                    max: (1 << (styleKeyValue <= Connection.NumberStyleKeyMax ? 31 : 15)) - 1,
                });
            }
        } else if (styleKeyValue >= Connection.ColorOpacityStyleKeyMin && styleKeyValue <= Connection.ColorOpacityStyleKeyMax) {
            return addStyleValue("menuValues/ColorMenuValue.qml", {
                attrKey: styleKeyValue,
                propName: styleKeyText,
                colorPicker: popup.colorPicker
            });
        } else if (styleKeyValue >= Connection.ByteStyleKeyMin && styleKeyValue <= Connection.ByteStyleKeyMax) {
            return addStyleValue("menuValues/IntMenuValue.qml", {
                attrKey: styleKeyValue,
                propName: styleKeyText,
                min: 0,
                max: (1 << 8) - 1,
            });
        } else if (styleKeyValue >= Connection.NonTypeStyleKeyMin && styleKeyValue <= Connection.NonTypeStyleKeyMax) {
            if (styleKeyText.startsWith("Font")) {
                return addStyleValue("menuValues/NonTypeMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    value: styleKeyValue,
                });
            } else if (styleKeyValue === Connection.AlignTopLeft) {
                let options = [];
                for (let i = Connection.AlignTopLeft; i <= Connection.AlignBottomRight; i++) {
                    options.push({ text: Connection.styleKeyString(i).slice(5), value: i });
                }
                return addStyleValue("menuValues/ValueOptionMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    options: options,
                });
            } else if (styleKeyValue === Connection.AlignTransformPivot) {
                return addStyleValue("menuValues/ValueOptionMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    options: [
                        { text: "This", value: Connection.AlignTransformPivot },
                        { text: "All", value: Connection.AlignTransformPivotAll },
                        { text: "Event", value: Connection.AlignTransformPivotEvent },
                        { text: "Event All", value: Connection.AlignTransformPivotEventAll },
                    ],
                });
            } else {
                let start = Connection.styleKeyString(styleKeyValue).replace(/^(.*)[A-Z][^A-Z]*$/, "$1");
                let options = [];
                let i = styleKeyValue;
                let styleKey = Connection.styleKeyString(i);
                do {
                    options.push({ text: styleKey.slice(start.length), value: i });
                    i++;
                    styleKey = Connection.styleKeyString(i);
                } while (styleKey.startsWith(start));
                return addStyleValue("menuValues/ValueOptionMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    options: options,
                });
            }
        }
    }
    function changeStyleSelector(): void {
        if (styleValueLayout.data.length !== 0)
            menuValueLayout.styleData[menuValueLayout.styleSelector] = styleValueLayout.data;
        else if (menuValueLayout.styleSelector in menuValueLayout.styleData)
            delete menuValueLayout.styleData[menuValueLayout.styleSelector];
        menuValueLayout.styleSelector = stateComboBox.currentValue | partComboBox.currentValue;
        clearStyleValues();
        if (menuValueLayout.styleSelector in menuValueLayout.styleData) {
            for (let styleData of menuValueLayout.styleData[menuValueLayout.styleSelector]) {
                let menuValue = addStyleKeyValue(styleData.attrKey, styleData.name);
                if (menuValue instanceof DirectionsMenuValue)
                    menuValue.value = styleData.value.join(";");
                else if (menuValue instanceof ColorMenuValue)
                    menuValue.value = styleData.value.length > 7 ? "#" + styleData.value.slice(7, 9) + styleData.value.slice(1, 7) : styleData.value;
                else if (!(menuValue instanceof NonTypeMenuValue))
                    menuValue.value = styleData.value;
            }
        }
    }
}