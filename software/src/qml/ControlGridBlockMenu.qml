import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls
import LvglSimulator

Popup {
    id: popup
    readonly property list<string> blockTypes: ["Button"]
    readonly property list<string> subWidgetTypes: ["Text", "Image"]
    required property Settings settings
    required property ControlGridQml controlGrid
    required property BigTextBox bigTextBox
    required property ColorPicker colorPicker
    required property ImagesMenu imagesMenu
    property bool newBlock
    property int row
    property int column
    property int rows: controlGrid.rows
    property int columns: controlGrid.columns
    property var blockStyleData: {}

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
                Layout.minimumWidth: childrenRect.width
                Layout.maximumHeight: childrenRect.height + 50

                LvglDisplay {
                    id: demoDisplayPanel
                    name: "DemoBlockDisplay"
                    implicitWidth: 225
                    implicitHeight: 125
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
            // Main Widget Configuration
            OptionMenuValue {
                id: typeMenuValue
                attrKey: "type"
                propName: "Type"
                options: popup.blockTypes
                visible: subWidgetsScroll.subIndex === 0
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
            }
            IntMenuValue {
                id: rowMenuValue
                attrKey: "row"
                propName: "Row"
                min: 0
                max: popup.rows - 1
                visible: subWidgetsScroll.subIndex === 0
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
            }
            IntMenuValue {
                id: columnMenuValue
                attrKey: "column"
                propName: "Column"
                min: 0
                max: popup.columns - 1
                visible: subWidgetsScroll.subIndex === 0
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
            }
            IntMenuValue {
                id: rowSpanMenuValue
                attrKey: "rowSpan"
                propName: "Row Span"
                min: 1
                max: popup.rows - rowMenuValue.numberValue
                visible: subWidgetsScroll.subIndex === 0
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
                onValueChanged: {
                    configureDemoDisplayCrop(rowSpanMenuValue.numberValue, columnSpanMenuValue.numberValue);
                    updateDemoDisplayLive();
                }
            }
            IntMenuValue {
                id: columnSpanMenuValue
                attrKey: "columnSpan"
                propName: "Column Span"
                min: 1
                max: popup.columns - columnMenuValue.numberValue
                visible: subWidgetsScroll.subIndex === 0
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
                onValueChanged: {
                    configureDemoDisplayCrop(rowSpanMenuValue.numberValue, columnSpanMenuValue.numberValue);
                    updateDemoDisplayLive();
                }
            }
            Label {
                text: "Sub Widgets:"
                font.pointSize: 13
                font.underline: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                visible: subWidgetsScroll.subIndex === 0
                Layout.fillWidth: true
                Layout.topMargin: 5
                Layout.bottomMargin: 5
            }
            ScrollView {
                id: subWidgetsScroll
                readonly property string attrKey: "subWidgets"
                property alias value: subWidgetsView.model
                property int subIndex: 0
                Layout.fillWidth: true
                Layout.fillHeight: true
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                visible: subWidgetsScroll.subIndex === 0
                clip: true

                ColumnLayout {
                    width: subWidgetsScroll.availableWidth
                    ListView {
                        id: subWidgetsView
                        Layout.fillWidth: true
                        Layout.preferredHeight: contentHeight
                        spacing: 5
                        clip: true
                        model: []
                        delegate: RowLayout {
                            required property int index
                            required property var model
                            height: 30
                            width: ListView.view.width
                            Label {
                                text: (parent.model.name && parent.model.name.length > 0 ? parent.model.name : parent.model.type) ?? ""
                                color: Theme.labelWhite
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                            }
                            Button {
                                Layout.fillHeight: true
                                Layout.preferredWidth: height
                                text: Theme.icons.edit
                                font.family: Theme.iconFontName
                                font.weight: Theme.iconFontWeight
                                font.pixelSize: Theme.iconFontSize
                                contentItem: Text {
                                    text: parent.text
                                    font: parent.font
                                    opacity: parent.enabled ? 1.0 : 0.3
                                    color: parent.down ? Theme.buttonBlueActive : Theme.buttonBlue
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                                    radius: Theme.buttonRadius
                                    border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                                    border.width: Theme.buttonBorderWidth
                                }
                                onClicked: popup.editSubWidget(index)
                            }
                            Button {
                                Layout.fillHeight: true
                                Layout.preferredWidth: height
                                text: Theme.icons.trash
                                font.family: Theme.iconFontName
                                font.weight: Theme.iconFontWeight
                                font.pixelSize: Theme.iconFontSize
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
                                onClicked: subWidgetsView.model.splice(index, 1);
                            }
                        }
                        onModelChanged: Qt.callLater(popup.updateDemoDisplayLive);
                    }
                    RowLayout {
                        id: subWidgetAdder
                        Layout.fillWidth: true
                        ComboBox {
                            id: subWidgetComboBox
                            Layout.fillWidth: true
                            model: popup.subWidgetTypes
                            background: Rectangle {
                                color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                                radius: Theme.buttonRadius
                                border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                                border.width: Theme.buttonBorderWidth
                            }
                            Component.onCompleted: currentIndex = -1
                        }
                        Button {
                            Layout.preferredWidth: 30
                            Layout.preferredHeight: 30
                            Layout.alignment: Qt.AlignCenter
                            text: Theme.icons.add
                            font.family: Theme.iconFontName
                            font.weight: Theme.iconFontWeight
                            font.pixelSize: Theme.iconFontSize
                            enabled: subWidgetComboBox.currentIndex !== -1
                            background: Rectangle {
                                color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                                radius: Theme.buttonRadius
                                border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                                border.width: Theme.buttonBorderWidth
                            }
                            onClicked: {
                                const subWidget = { type: subWidgetComboBox.currentText };
                                if (subWidgetComboBox.currentText === "Image")
                                    subWidget.style = { 0: [{ attrKey: Connection.Width, name: "Size", text: "100p%", value: 100 }]}
                                subWidgetsView.model.push(subWidget);
                                popup.updateDemoDisplayLive();
                                popup.scrollSubWidgetsToBottom();
                            }
                        }
                    }
                }
            }
            // Sub Widget Configuration
            StringMenuValue {
                id: subNameMenuValue
                attrKey: "name"
                propName: "Name"
                visible: subWidgetsScroll.subIndex !== 0
                Layout.minimumHeight: 30
                Layout.maximumHeight: 30
            }
            Loader {
                id: subWidgetValueLoader
                property var data: {
                    if (!active) return;
                    let data = {};
                    for (let menuValue of subWidgetValueLoader.item.children) {
                        if (menuValue.attrKey === undefined || !menuValue.visible) continue;
                        if (menuValue instanceof IntMenuValue) {
                            data[menuValue.attrKey] = menuValue.numberValue;
                        } else {
                            data[menuValue.attrKey] = menuValue.value;
                        }
                    }
                    return data;
                }
                active: subWidgetsScroll.subIndex !== 0
                Layout.fillWidth: true
                Layout.maximumHeight: active ? Number.POSITIVE_INFINITY : 0
                onDataChanged: {
                    Qt.callLater(popup.updateDemoDisplayLive)
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumWidth: removeButton.implicitWidth + 10 + saveButton.implicitWidth + 10 + backButton.implicitWidth
                Layout.minimumHeight: Math.max(removeButton.implicitHeight, Math.max(saveButton.implicitHeight, backButton.implicitHeight))
                Button {
                    id: backButton
                    implicitHeight: 30
                    horizontalPadding: 10
                    text: Theme.icons.back
                    font.family: Theme.iconFontName
                    font.weight: Theme.iconFontWeight
                    font.pixelSize: Theme.iconFontSize
                    anchors.bottom: parent.bottom
                    visible: subWidgetsScroll.subIndex !== 0
                    anchors.right: removeButton.left
                    anchors.rightMargin: 10
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        opacity: parent.enabled ? 1.0 : 0.3
                        color: parent.down ? Theme.buttonBlueActive : Theme.buttonBlue
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                        radius: Theme.buttonRadius
                        border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                        border.width: Theme.buttonBorderWidth
                    }

                    onClicked: popup.returnToMainWidget()
                }
                Button {
                    id: removeButton
                    implicitHeight: 30
                    horizontalPadding: 10
                    text: Theme.icons.trash
                    font.family: Theme.iconFontName
                    font.weight: Theme.iconFontWeight
                    font.pixelSize: Theme.iconFontSize
                    enabled: !popup.newBlock && subWidgetsScroll.subIndex === 0
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
                    enabled: subWidgetsScroll.subIndex === 0
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
        styleDataView.macros = {};
        failLabel.text = "";
        failLabel.visible = false;
        colorPicker.x = Qt.binding(() => Math.min(Overlay.overlay.width - colorPicker.width, popup.x + popup.width));
        colorPicker.y = Qt.binding(() => popup.y + (popup.height - colorPicker.height) / 2);
        let blockData = newBlock ? undefined : settings.loadBlock(controlGrid.layoutName, row, column);
        if (blockData !== undefined)
            typeMenuValue.value = blockData["type"];
        else
            typeMenuValue.value = popup.blockTypes[0];
        demoControlGrid.setLayout(rows, columns);
        demoControlGrid.outerPad = controlGrid.outerPad;
        demoControlGrid.rowPad = controlGrid.rowPad;
        demoControlGrid.columnPad = controlGrid.columnPad;
        demoControlGrid.clearImages();
        loadBlock(blockData);
        configureDemoDisplayCrop(blockData ? blockData.rowSpan : 1, blockData ? blockData.columnSpan : 1);
        loadDemoDisplay(blockData ? blockData.rowSpan : 1, blockData ? blockData.columnSpan : 1);
    }

    onClosed: {
        returnToMainWidget();
        demoControlGrid.clear();
    }

    function configureDemoDisplayCrop(rowSpan: int, columnSpan: int): void {
        demoDisplayPanel.imageClipRect = Qt.rect(
            ((demoDisplayPanel.displayWidth - controlGrid.controlGridWidth) / 2) - controlGrid.columnPad,
            ((demoDisplayPanel.displayHeight - controlGrid.controlGridHeight) / 2) - controlGrid.rowPad,
            (((controlGrid.controlGridWidth + controlGrid.columnPad) / columns) * columnSpan) + controlGrid.columnPad,
            (((controlGrid.controlGridHeight + controlGrid.rowPad) / rows) * rowSpan) + controlGrid.rowPad
        );
        if (demoDisplayPanel.imageClipRect.width / demoDisplayPanel.imageClipRect.height * 125 <= 225) {
            demoDisplayPanel.implicitWidth = demoDisplayPanel.imageClipRect.width / demoDisplayPanel.imageClipRect.height * 125;
            demoDisplayPanel.implicitHeight = 125;
        } else {
            demoDisplayPanel.implicitWidth = 225;
            demoDisplayPanel.implicitHeight = demoDisplayPanel.imageClipRect.height / demoDisplayPanel.imageClipRect.width * 225;
        }
    }

    function loadDemoDisplay(rowSpan: int, columnSpan: int): void {
        demoControlGrid.remove(0, 0);
        demoControlGrid.addWidget(typeMenuValue.value, 0, ((rowSpan-1) * columns) + (columnSpan-1));
        for (const styleSelector in blockStyleData) {
            demoControlGrid.setStyle(0, 0, styleSelector, blockStyleData[styleSelector]);
        }
        for (let i = 0; i < subWidgetsScroll.value.length; i++) {
            const subWidget = subWidgetsScroll.value[i];
            if (subWidget.type === "Image" && (!("image" in subWidget) || !("imageKey" in subWidget.image)
                || !subWidget.image.imageKey || subWidget.image.imageKey.length <= 1)) continue;
            const subIndex = demoControlGrid.subWidget(subWidget.type, 0, 0, subWidget);
            if (subIndex !== i+1) {
                console.error(`Expected Sub Widget to placed at index ${i+1}, but was placed at ${subIndex}`);
                return;
            }
            for (const styleSelector in subWidget.style) {
                demoControlGrid.setStyle(0, subIndex, styleSelector, subWidget.style[styleSelector]);
            }
        }
    }
    function updateImagesLive(): void {
        let images = new Set();
        for (const styleSelector in blockStyleData) {
            for (const styleElement of blockStyleData[styleSelector]) {
                if (styleElement.attrKey === Connection.BackgroundImageIndex
                    && styleElement.value && styleElement.value.imageKey && styleElement.value.imageKey.length > 1)
                    images.add(styleElement.value.imageKey);
            }
        }
        for (let i = 1; i <= subWidgetsScroll.value.length; i++) {
            const subWidget = subWidgetsScroll.subIndex === i ?
                    Object.assign({}, subWidgetsScroll.value[i-1], subWidgetValueLoader.data) :
                    subWidgetsScroll.value[i-1];
            if (subWidget.type === "Image" && subWidget.image && subWidget.image.imageKey)
                images.add(subWidget.image.imageKey);
            const subWidgetStyleData = subWidgetsScroll.subIndex === i ? styleDataView.styleData : subWidget.style;
            for (const styleSelector in subWidgetStyleData) {
                for (const styleElement of subWidgetStyleData[styleSelector]) {
                    if (styleElement.attrKey === Connection.BackgroundImageIndex
                        && styleElement.value && styleElement.value.imageKey && styleElement.value.imageKey.length > 1)
                        images.add(styleElement.value.imageKey);
                }
            }
        }
        demoControlGrid.loadImages(Array.from(images));
    }
    function updateDemoDisplayLive(): void {
        if (!popup.visible) return;
        demoControlGrid.remove(0, 0);
        demoControlGrid.addWidget(typeMenuValue.value, 0, ((rowSpanMenuValue.numberValue-1) * columns) + (columnSpanMenuValue.numberValue-1));
        for (let i = 0; i < subWidgetsScroll.value.length; i++) {
            const subWidget = subWidgetsScroll.subIndex === i + 1 ?
                Object.assign({}, subWidgetsScroll.value[i], subWidgetValueLoader.data) :
                subWidgetsScroll.value[i];
            if (subWidget.type === "Image" && (!subWidget.image || !subWidget.image.imageKey || subWidget.image.imageKey.length <= 1)) continue;
            const subIndex = demoControlGrid.subWidget(subWidget.type, 0, 0, subWidget);
            if (subIndex !== i + 1) {
                console.error(`Expected Sub Widget to placed at index ${i + 1}, but was placed at ${subIndex}`);
                return;
            }
        }

        buildMacros();
        styleDataView.revalidateStyleValue();
        updateImagesLive();

        for (const styleSelector in blockStyleData) {
            demoControlGrid.setStyle(0, 0, styleSelector, blockStyleData[styleSelector]);
        }
        for (let i = 0; i < subWidgetsScroll.value.length; i++) {
            const subIndex = i + 1;
            const subWidget = subWidgetsScroll.subIndex === subIndex ?
                Object.assign({}, subWidgetsScroll.value[i], subWidgetValueLoader.data) :
                subWidgetsScroll.value[i];
            if (subWidget.type === "Image" && (!subWidget.image || !subWidget.image.imageKey || subWidget.image.imageKey.length <= 1)) continue;
            const subWidgetStyleData = subWidgetsScroll.subIndex === subIndex ? styleDataView.styleData : subWidget.style;
            for (const styleSelector in subWidgetStyleData) {
                demoControlGrid.setStyle(0, subIndex, styleSelector, subWidgetStyleData[styleSelector]);
            }
        }
    }
    function loadBlock(data): void {
        if (!newBlock) {
            blockStyleData = data.style ?? {};
            styleDataView.styleData = Qt.binding(() => blockStyleData);
            for (let menuValue of rightLayout.children) {
                if (menuValue.attrKey === undefined || !menuValue.visible) continue;
                menuValue.value = data[menuValue.attrKey];
            }
            if (!subWidgetsScroll.value) subWidgetsScroll.value = [];
        } else {
            blockStyleData = {};
            styleDataView.styleData = Qt.binding(() => blockStyleData);
            rowSpanMenuValue.value = 1;
            columnSpanMenuValue.value = 1;
            rowMenuValue.value = row;
            columnMenuValue.value = column;
            subWidgetsScroll.value = [];
        }
    }
    function saveBlock(): void {
        styleDataView.updateStyleData();
        if (subWidgetsScroll.subIndex !== 0)
            subWidgetsView.model[subWidgetsScroll.subIndex-1].style = styleDataView.styleData;
        let data = {
            style: blockStyleData,
        };
        for (let menuValue of rightLayout.children) {
            if (menuValue.attrKey === undefined || !menuValue.visible) continue;
            if (menuValue instanceof IntMenuValue)
                data[menuValue.attrKey] = menuValue.numberValue;
            else
                data[menuValue.attrKey] = menuValue.value;
        }
        if (newBlock) {
            controlGrid.updateImages(-1, -1, data);
        } else {
            controlGrid.removeWidget((row * columns) + column, 0);
            controlGrid.updateImages(row, column, data);
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
            controlGrid.updateImages();
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
        controlGrid.updateImages();
        controlGrid.removeWidget((row * controlGrid.columns) + column, 0);
        popup.close();
    }

    function buildMacros(): void {
        const mainMacros = {};
        const sizePosData = { index: 0, subIndex: subWidgetsScroll.subIndex };
        demoControlGrid.insertCoordsData(sizePosData);
        Utils.buildMacros(
            mainMacros,
            styleDataView.styleData,
            sizePosData.width,
            sizePosData.height,
            rowMenuValue.numberValue,
            columnMenuValue.numberValue,
            rowSpanMenuValue.numberValue,
            columnSpanMenuValue.numberValue
        );
        if (subWidgetsScroll.subIndex !== 0) {
            const parentMacros = {};
            const parentSizePosData = { index: 0 };
            demoControlGrid.insertCoordsData(parentSizePosData);
            Utils.buildMacros(
                parentMacros,
                blockStyleData,
                parentSizePosData.width,
                parentSizePosData.height,
                rowMenuValue.numberValue,
                columnMenuValue.numberValue,
                rowSpanMenuValue.numberValue,
                columnSpanMenuValue.numberValue
            );
            mainMacros.parent = parentMacros;
        }
        styleDataView.macros = mainMacros;
    }

    // Sub Widgets Layout
    function scrollSubWidgetsToBottom(): void {
        subWidgetsScroll.contentItem.contentY = subWidgetsScroll.contentItem.contentHeight - subWidgetsScroll.contentItem.height + 100;
    }
    function editSubWidget(index: int): void {
        if (subWidgetsScroll.subIndex !== 0)
            returnToMainWidget();
        styleDataView.updateStyleData();
        if (subWidgetsScroll.subIndex !== 0)
            subWidgetsView.model[subWidgetsScroll.subIndex-1].style = styleDataView.styleData;
        if (!subWidgetsView.model[index].style)
            subWidgetsView.model[index].style = {};
        subWidgetValueLoader.setSource("subWidgetValues/Sub" + subWidgetsView.model[index].type + "Value.qml",
                subWidgetsView.model[index].type === "Image" ?
                    { imagesMenu: popup.imagesMenu, macros: Qt.binding(() => styleDataView.macros) } : {});
        subWidgetsScroll.subIndex = index + 1;
        subNameMenuValue.value = subWidgetsView.model[index].name ?? "";
        for (let menuValue of subWidgetValueLoader.item.children) {
            if (menuValue.attrKey === undefined || !menuValue.visible) continue;
            if (!(menuValue.attrKey in subWidgetsView.model[index]) && typeof menuValue.value === "string") {
                if (!(menuValue instanceof IntMenuValue))
                    menuValue.value = "";
                continue;
            }
            menuValue.value = subWidgetsView.model[index][menuValue.attrKey];
        }
        styleDataView.styleData = Qt.binding(() => subWidgetsView.model[index].style);
    }

    function returnToMainWidget(): void {
        if (subWidgetsScroll.subIndex === 0) return;
        const index = subWidgetsScroll.subIndex - 1;
        if (subNameMenuValue.value.length === 0)
            delete subWidgetsView.model[index].name;
        else
            subWidgetsView.model[index].name = subNameMenuValue.value;
        for (let menuValue of subWidgetValueLoader.item.children) {
            if (menuValue.attrKey === undefined || !menuValue.visible) continue;
            if (menuValue instanceof IntMenuValue) {
                subWidgetsView.model[index][menuValue.attrKey] = menuValue.numberValue;
            } else {
                subWidgetsView.model[index][menuValue.attrKey] = menuValue.value;
            }
        }
        styleDataView.updateStyleData();
        subWidgetsView.model[index].style = styleDataView.styleData;
        styleDataView.styleData = Qt.binding(() => blockStyleData);
        subWidgetsScroll.subIndex = 0;
    }
}