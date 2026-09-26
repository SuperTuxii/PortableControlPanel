import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ScrollView {
    id: menuValueScroll
    signal styleDataUpdated()
    required property BigTextBox bigTextBox
    required property ColorPicker colorPicker
    required property ImagesMenu imagesMenu
    property var macros: {}
    property var styleData: {}
    property int styleSelector: 0
    Layout.fillWidth: true
    Layout.fillHeight: true
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
    clip: true

    ColumnLayout {
        id: menuValueLayout

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
                    if (menuValueScroll.styleSelector !== (stateComboBox.currentValue | partComboBox.currentValue))
                        menuValueScroll.changeStyleSelector();
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
                    if (menuValueScroll.styleSelector !== (stateComboBox.currentValue | partComboBox.currentValue))
                        menuValueScroll.changeStyleSelector();
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
                            data.push({ attrKey: parseInt(child.attrKey), name: child.propName, value: Array.from(child.numberValues), text: child.value });
                        else if (child instanceof ColorMenuValue)
                            data.push({ attrKey: parseInt(child.attrKey), name: child.propName, value: (child.value.a === 1 ? child.value.toString() + "FF" : "#" + child.value.toString().slice(3, 9) + child.value.toString().slice(1, 3)).toUpperCase() });
                        else if (child instanceof IntMenuValue)
                            data.push({ attrKey: parseInt(child.attrKey), name: child.propName, value: child.numberValue, text: child.value });
                        else
                            data.push({ attrKey: parseInt(child.attrKey), name: child.propName, value: child.value });
                    }
                    return data;
                }
                property bool dirty: false
                onDataChanged: {
                    if (!dirty) {
                        dirty = true;
                        Qt.callLater(menuValueScroll.updateStyleData)
                    }
                }
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
            Layout.preferredWidth: menuValueScroll.availableWidth
            Layout.fillWidth: true
            onAddStyleKeyValue: (styleKeyValue, styleKeyText) => {
                menuValueScroll.addStyleKeyValue(styleKeyValue, styleKeyText);
                menuValueScroll.scrollToBottom();
            }
        }
    }

    Component.onCompleted: styleSelector = stateComboBox.currentValue | partComboBox.currentValue

    onStyleDataChanged: {
        stateComboBox.currentIndex = 0;
        partComboBox.currentIndex = 0;
        styleValueLayout.dirty = false;
        changeStyleSelector();
    }

    function scrollToBottom(): void {
        contentItem.contentY = contentItem.contentHeight - contentItem.height + 100;
    }
    function updateStyleData(): void {
        if (styleData) {
            if (styleValueLayout.data.length !== 0)
                styleData[styleSelector] = styleValueLayout.data;
            else if (styleSelector in styleData)
                delete styleData[styleSelector];
        }
        styleValueLayout.dirty = false;
        styleDataUpdated();
    }
    function changeStyleSelector(): void {
        if (styleValueLayout.dirty)
            updateStyleData();
        styleSelector = stateComboBox.currentValue | partComboBox.currentValue;
        clearStyleValues();
        if (styleSelector in styleData) {
            for (let styleValue of styleData[styleSelector]) {
                let menuValue = addStyleKeyValue(styleValue.attrKey, styleValue.name);
                if (menuValue instanceof DirectionsMenuValue) {
                    menuValue.numberValues = styleValue.value;
                    menuValue.value = styleValue.text;
                } else if (menuValue instanceof ColorMenuValue) {
                    menuValue.value = styleValue.value.length > 7 ? "#" + styleValue.value.slice(7, 9) + styleValue.value.slice(1, 7) : styleValue.value;
                } else if (menuValue instanceof IntMenuValue) {
                    menuValue.numberValue = styleValue.value;
                    menuValue.value = styleValue.text;
                } else if (!(menuValue instanceof NonTypeMenuValue)) {
                    menuValue.value = styleValue.value;
                }
            }
        }
    }
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
            let directions = Utils.getStyleKeyDirections(styleKeyValue);
            if (directions > 0) {
                return addStyleValue("menuValues/DirectionsMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    directions: directions,
                    min: -(1 << (styleKeyValue <= Connection.NumberStyleKeyMax ? 31 : 15)),
                    max: (1 << (styleKeyValue <= Connection.NumberStyleKeyMax ? 31 : 15)) - 1,
                    preprocessor: (string) => Utils.macroPreprocessor(macros, string, styleSelector, true),
                    bigTextBox: bigTextBox,
                });
            } else {
                return addStyleValue("menuValues/IntMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    min: -(1 << (styleKeyValue <= Connection.NumberStyleKeyMax ? 31 : 15)),
                    max: (1 << (styleKeyValue <= Connection.NumberStyleKeyMax ? 31 : 15)) - 1,
                    preprocessor: (string) => Utils.macroPreprocessor(macros, string, styleSelector),
                    bigTextBox: bigTextBox,
                });
            }
        } else if (styleKeyValue >= Connection.ColorOpacityStyleKeyMin && styleKeyValue <= Connection.ColorOpacityStyleKeyMax) {
            return addStyleValue("menuValues/ColorMenuValue.qml", {
                attrKey: styleKeyValue,
                propName: styleKeyText,
                colorPicker: colorPicker
            });
        } else if (styleKeyValue >= Connection.ByteStyleKeyMin && styleKeyValue <= Connection.ByteStyleKeyMax) {
            if (styleKeyValue === Connection.BackgroundImageIndex) {
                return addStyleValue("menuValues/ImageMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    imagesMenu: imagesMenu,
                    macros: Qt.binding(() => macros),
                    styleSelector: Qt.binding(() => styleSelector)
                });
            } else {
                return addStyleValue("menuValues/IntMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    min: 0,
                    max: (1 << 8) - 1,
                    preprocessor: (string) => Utils.macroPreprocessor(macros, string, styleSelector),
                    bigTextBox: bigTextBox,
                });
            }
        } else if (styleKeyValue >= Connection.NonTypeStyleKeyMin && styleKeyValue <= Connection.NonTypeStyleKeyMax) {
            if (styleKeyText.startsWith("Font")) {
                return addStyleValue("menuValues/NonTypeMenuValue.qml", {
                    attrKey: styleKeyValue,
                    propName: styleKeyText,
                    value: styleKeyValue,
                });
            } else if (styleKeyValue === Connection.AlignTopLeft) {
                let options = [];
                for (let i = Connection.AlignTopLeft; i <= Connection.AlignCenter; i++) {
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
    function revalidateStyleValue(): void {
        for (let refreshes = 0; refreshes < 10; refreshes++) {
            let changed = false;
            for (let child of styleValueLayout.children) {
                if (child === styleValuePlaceholder) {
                    if (!child.reference) continue;
                    child = child.reference;
                }
                if (child instanceof DirectionsMenuValue) {
                    let oldValue = Array.from(child.numberValues);
                    child.revalidate();
                    if (oldValue.length !== child.numberValues.length
                        || !oldValue.every((value, index) => value === child.numberValues[index])) {
                        macros.style[styleSelector][child.propName.toLowerCase()] = child.numberValues.length === 1 ? child.numberValues[0] : child.numberValues;
                        changed = true;
                    }
                } else if (child instanceof IntMenuValue) {
                    let oldValue = child.numberValue;
                    child.revalidate();
                    if (oldValue !== child.numberValue) {
                        macros.style[styleSelector][child.propName.toLowerCase()] = child.numberValue;
                        changed = true;
                    }
                } else if (child instanceof ImageMenuValue) {
                    changed |= Utils.refreshImageValue(macros, child.value, styleSelector);
                }
            }
            changed = changed || Utils.refreshStyleDataSingle(macros, styleData, styleSelector);
            if (!changed) return;
        }

        for (let child of styleValueLayout.children) {
            if (child === styleValuePlaceholder) {
                if (!child.reference) continue;
                child = child.reference;
            }
            if (child instanceof DirectionsMenuValue) {
                let oldValue = Array.from(child.numberValues);
                child.revalidate();
                if (oldValue.length !== child.numberValues.length
                    || !oldValue.every((value, index) => value === child.numberValues[index])) {
                    child.numberValues = [child.min > 0 ? child.min : 0];
                    delete macros.style[styleSelector][child.propName.toLowerCase()];
                    child.revalidate();
                    child.valid = false;
                }
            } else if (child instanceof IntMenuValue) {
                let oldValue = child.numberValue;
                child.revalidate();
                if (oldValue !== child.numberValue) {
                    child.numberValue = child.min > 0 ? child.min : 0
                    delete macros.style[styleSelector][child.propName.toLowerCase()];
                    child.revalidate();
                    child.valid = false;
                }
            }
        }
        Utils.tooManyStyleDataRecursions(macros, styleData, styleSelector);
    }
}