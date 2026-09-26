import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Popup {
    id: popup

    required property Settings settings
    required property BigTextBox bigTextBox
    property list<url> importQueue
    property bool selectable: false
    property var selected: undefined
    property var selectedObject
    property size selectedSize: Qt.size(1, 1)
    property var macros: undefined
    property int styleSelector: 0

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

    DropArea {
        anchors.fill: parent

        onDropped: (event) => {
            if (event.hasUrls) {
                for (let url of event.urls) {
                    Connection.cacheImage(url);
                }
            }
        }
    }

    ScrollView {
        id: imagesScroll
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: modMenu.left
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        clip: true

        GridView {
            id: imagesView
            width: imagesScroll.availableWidth
            cellWidth: 200
            cellHeight: 200
            clip: true
            model: settings.loadImages()
            delegate: Item {
                id: root
                required property var model
                width: imagesView.cellWidth
                height: imagesView.cellHeight

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 5
                    color: Qt.darker(Theme.mainBackground, popup.isSelected(model.key) ? Theme.buttonBackgroundDarker : hover.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                    border.width: Theme.borderWidth
                    border.color: Theme.border
                    radius: Theme.borderRadius
                    ToolTip.text: model.key
                    ToolTip.visible: hover.hovered
                    ToolTip.delay: 500

                    Image {
                        id: image
                        property size initialSize: Qt.size(0, 0)
                        property real shrinkAmount: Math.min((parent.width - Theme.borderRadius) / sourceSize.width, (parent.height - Theme.borderRadius) / sourceSize.height)
                        width: sourceSize.width * shrinkAmount
                        height: sourceSize.height * shrinkAmount
                        anchors.centerIn: parent
                        source: createSource()
                        asynchronous: true
                        retainWhileLoading: true
                        smooth: false

                        Component.onCompleted: {
                            if (model.initialSize)
                                initialSize = Qt.size(model.initialSize[0], model.initialSize[1]);
                            root.checkImageSize();
                            if (popup.isSelected(model.key)) return;
                            root.recalcAttributes();
                            root.removeBindings();
                        }

                        function createSource() {
                            return "image://cached/" + Utils.createImageKey(model, ["cropPos", "cropSize", "colorFormat"]);
                        }
                    }

                    Button {
                        width: Theme.iconFontSize + 6
                        height: Theme.iconFontSize + 6
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: 5
                        visible: hover.hovered && !popup.selectable
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
                            opacity: 0.5
                            color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                            radius: Theme.buttonRadius
                            border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                            border.width: Theme.buttonBorderWidth
                        }
                        onClicked: popup.settings.removeImage(model.key)
                    }

                    Button {
                        width: Theme.iconFontSize + 6
                        height: Theme.iconFontSize + 6
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 5
                        visible: hover.hovered && !popup.selectable
                        text: Theme.icons.refresh
                        font.family: Theme.iconFontName
                        font.weight: Theme.iconFontWeight
                        font.pixelSize: Theme.iconFontSize
                        background: Rectangle {
                            opacity: 0.5
                            color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                            radius: Theme.buttonRadius
                            border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                            border.width: Theme.buttonBorderWidth
                        }
                        onClicked: {
                            const successHandler = (url, filename, path) => {
                                if (url === root.model.url) {
                                    image.source = "";
                                    image.sourceSize = undefined;
                                    image.source = Qt.binding(image.createSource);
                                    root.checkImageSize();
                                    root.recalcAttributes();
                                    root.removeBindings();
                                    Connection.imageCached.disconnect(successHandler);
                                    Connection.imageCachingFailed.disconnect(failHandler);
                                }
                            };
                            const failHandler = (url, error) => {
                                if (url === root.model.url) {
                                    Connection.imageCached.disconnect(successHandler);
                                    Connection.imageCachingFailed.disconnect(failHandler);
                                }
                            };
                            Connection.imageCached.connect(successHandler);
                            Connection.imageCachingFailed.connect(failHandler);
                            Connection.cacheImage(model.url, model.path);
                        }
                    }

                    TapHandler {
                        onTapped: {
                            const index = model.index;
                            const view = imagesView;
                            if (popup.selectable) {
                                if (popup.selectedObject)
                                    imagesView.itemAtIndex(popup.selectedObject.index).removeBindings();
                            } else {
                                modLayout.saveMenuValues();
                            }
                            view.itemAtIndex(index).select();
                        }
                    }

                    HoverHandler {
                        id: hover
                    }
                }

                Component.onCompleted: {
                    if (popup.isSelected(model.key)) {
                        const selected = popup.selected;
                        popup.selected = undefined;
                        select();
                        modLayout.loadMenuValues(selected, false);
                    }
                }

                function select() {
                    if (!popup.isSelected(model.key)) {
                        popup.selected = { key: model.key };
                        popup.selectedObject = model;
                        popup.selectedSize = Qt.binding(() => image.initialSize);
                        modLayout.loadMenuValues(model);
                        image.sourceSize = Qt.binding(() => modLayout.resize);
                        model.cropPos = Qt.binding(() => cropPosMenuValue.numberValues);
                        model.cropSize = Qt.binding(() => cropSizeMenuValue.numberValues);
                    } else {
                        popup.selected = undefined;
                        popup.selectedObject = undefined;
                    }
                }
                function checkImageSize() {
                    if (!model.path || !model.key || !image) return;
                    const imageSize = Connection.imageSize(model.path);
                    if (image.initialSize.width !== imageSize.width || image.initialSize.height !== imageSize.height)
                        popup.settings.editImage(model.key, {initialSize: [imageSize.width, imageSize.height]});
                }
                function recalcAttributes() {
                    if (!model.cropPosText || !model.cropSizeText || !model.resizeText) return;
                    const cropSize = Utils.parseDirectionsCalc(
                        Utils.macroPreprocessor({
                            width: image.initialSize.width, w: image.initialSize.width,
                            height: image.initialSize.height, h: image.initialSize.height
                        }, model.cropSizeText, 0, true),
                        2, 1, Math.max(image.initialSize.width, image.initialSize.height)
                    );
                    if (!cropSize) return;
                    const cropHeight = cropSize.length === 2 ? cropSize[1] : cropSize[0];
                    const cropPos = Utils.parseDirectionsCalc(
                        Utils.macroPreprocessor({
                            width: image.initialSize.width - cropSize[0],
                            w: image.initialSize.width - cropSize[0],
                            height: image.initialSize.height - cropHeight,
                            h: image.initialSize.height - cropHeight,
                        }, model.cropPosText, 0, true),
                        2, 0, Math.max(image.initialSize.width, image.initialSize.height)
                    );
                    const resize = Utils.parseDirectionsCalc(
                        Utils.macroPreprocessor({
                            width: cropSize[0], w: cropSize[0],
                            height: cropHeight, h: cropHeight
                        }, model.resizeText, 0, true),
                        2, 1, Math.max(image.initialSize.width, image.initialSize.height)
                    );
                    if (!cropPos || !resize) return;
                    if (model.cropPos && model.cropSize && model.resize && model.cropPos.length === cropPos.length
                        && model.cropSize.length === cropSize.length && model.resize.length === resize.length
                        && cropPos.every((value, i) => model.cropPos[i] === parseInt(value))
                        && cropSize.every((value, i) => model.cropSize[i] === parseInt(value))
                        && resize.every((value, i) => model.resize[i] === parseInt(value))) return;
                    popup.settings.editImage(model.key, {
                        cropPos: cropPos.map(item => parseInt(item)),
                        cropSize: cropSize.map(item => parseInt(item)),
                        resize: resize.map(item => parseInt(item))
                    });
                }
                function removeBindings() {
                    if (popup.selectable) {
                        const imageData = popup.settings.loadImage(model.key);
                        model.resize = imageData.resize;
                        model.cropPos = imageData.cropPos;
                        model.cropSize = imageData.cropSize;
                    } else {
                        model.cropPos = model.cropPos;
                        model.cropSize = model.cropSize;
                    }
                    if (model.cropPos && model.cropSize && model.resize)
                        image.sourceSize = Qt.size(
                            model.resize[0],
                            model.resize.length === 2 ? model.resize[1] : model.resize[0]
                        );
                    else
                        image.sourceSize = undefined;
                }
            }
        }
    }

    Rectangle {
        id: modMenu
        x: popup.width - popup.padding - width
        y: -popup.padding
        height: popup.height
        width: popup.selected ? popup.width / 3.2 : 0
        visible: popup.selected !== undefined
        color: Theme.mainBackground
        border.width: Theme.borderWidth
        border.color: Theme.border
        radius: Theme.borderRadius

        ColumnLayout {
            id: modLayout
            property rect cropRect: Qt.rect(
                cropPosMenuValue.numberValues[0],
                cropPosMenuValue.numberValues.length === 2 ? cropPosMenuValue.numberValues[1] : cropPosMenuValue.numberValues[0],
                cropSizeMenuValue.cropWidth,
                cropSizeMenuValue.cropHeight
            )
            property size resize: Qt.size(
                resizeMenuValue.numberValues[0],
                resizeMenuValue.numberValues.length === 2 ? resizeMenuValue.numberValues[1] : resizeMenuValue.numberValues[0]
            )
            anchors.fill: parent
            anchors.margins: Theme.borderRadius

            Label {
                text: "Presets:"
                visible: popup.selectable
                font.pointSize: 13
                font.underline: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.bottomMargin: 5
            }
            ComboBox {
                id: presetsComboBox
                readonly property string attrKey: "preset"
                property alias value: presetsComboBox.currentValue
                visible: popup.selectable
                Layout.preferredHeight: 30
                Layout.fillWidth: true
                model: ["Custom", "Image Config", "Fill Widget", "Fit Widget", "Cover Widget"]
                background: Rectangle {
                    color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                    radius: Theme.buttonRadius
                    border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                    border.width: Theme.buttonBorderWidth
                }
                onCurrentValueChanged: updatePreset()

                function updatePreset(): void {
                    resizeMenuValue.enabled = currentIndex <= 0;
                    cropPosMenuValue.enabled = currentIndex !== 1;
                    cropSizeMenuValue.enabled = currentIndex !== 1;
                    colorFormatMenuValue.enabled = currentIndex !== 1;
                    if (currentValue === "Image Config") {
                        const imageData = popup.settings.loadImage(popup.selected.key);
                        cropPosMenuValue.value = imageData.cropPosText;
                        cropSizeMenuValue.value = imageData.cropSizeText;
                        resizeMenuValue.value = imageData.resizeText;
                        colorFormatMenuValue.value = imageData.colorFormat;
                    } else if (currentValue === "Fill Widget") {
                        resizeMenuValue.value = "$(ww);$(wh)";
                    } else if (currentValue === "Fit Widget") {
                        resizeMenuValue.value = "($(ww)/$(w)) < ($(wh)/$(h)) ?\n$(ww) :\n$(w)*($(wh)/$(h));\n\n($(ww)/$(w)) < ($(wh)/$(h)) ?\n$(h)*($(ww)/$(w)) :\n$(wh)";
                    } else if (currentValue === "Cover Widget") {
                        resizeMenuValue.value = "($(ww)/$(w)) > ($(wh)/$(h)) ?\n$(ww) :\n$(w)*($(wh)/$(h));\n\n($(ww)/$(w)) > ($(wh)/$(h)) ?\n$(h)*($(ww)/$(w)) :\n$(wh)";
                    }
                }
            }
            Label {
                text: "Crop:"
                font.pointSize: 13
                font.underline: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.bottomMargin: 5
            }
            DirectionsMenuValue {
                id: cropPosMenuValue
                attrKey: "cropPos"
                propName: "Position"
                directions: 2
                min: 0
                max: Math.max(popup.selectedSize.width, popup.selectedSize.height)
                preprocessor: (string) => Utils.macroPreprocessor(
                    Utils.combineSizeMacros(
                        popup.macros,
                        popup.selectedSize.width - cropSizeMenuValue.cropWidth,
                        popup.selectedSize.height - cropSizeMenuValue.cropHeight
                    ), string, popup.styleSelector, true
                )
                bigTextBox: popup.bigTextBox
                Layout.preferredHeight: 30
                Layout.fillWidth: true
                Layout.fillHeight: false
            }
            DirectionsMenuValue {
                id: cropSizeMenuValue
                property var cropWidth: numberValues[0]
                property var cropHeight: numberValues.length === 2 ? numberValues[1] : numberValues[0]

                attrKey: "cropSize"
                propName: "Size"
                directions: 2
                min: 1
                max: Math.max(popup.selectedSize.width, popup.selectedSize.height)
                preprocessor: (string) => Utils.macroPreprocessor(
                    Utils.combineSizeMacros(
                        popup.macros,
                        popup.selectedSize.width,
                        popup.selectedSize.height
                    ), string, popup.styleSelector, true
                )
                bigTextBox: popup.bigTextBox
                Layout.preferredHeight: 30
                Layout.fillWidth: true
                Layout.fillHeight: false

                onCropWidthChanged: {
                    Qt.callLater(cropPosMenuValue.revalidate);
                    Qt.callLater(resizeMenuValue.revalidate);
                }
                onCropHeightChanged: {
                    Qt.callLater(cropPosMenuValue.revalidate);
                    Qt.callLater(resizeMenuValue.revalidate);
                }
            }
            Label {
                text: "Resize:"
                font.pointSize: 13
                font.underline: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.bottomMargin: 5
            }
            DirectionsMenuValue {
                id: resizeMenuValue
                attrKey: "resize"
                propName: "Dimensions"
                directions: 2
                min: 1
                max: Math.max(popup.selectedSize.width, popup.selectedSize.height, 1000)
                preprocessor: (string) => Utils.macroPreprocessor(
                    Utils.combineSizeMacros(
                        popup.macros,
                        modLayout.cropRect.width,
                        modLayout.cropRect.height
                    ), string, popup.styleSelector, true
                )
                bigTextBox: popup.bigTextBox
                Layout.preferredHeight: 30
                Layout.fillWidth: true
                Layout.fillHeight: false
            }
            Label {
                text: "Color Format:"
                font.pointSize: 13
                font.underline: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                Layout.fillHeight: false
                Layout.bottomMargin: 5
            }
            ValueOptionMenuValue {
                id: colorFormatMenuValue
                attrKey: "colorFormat"
                propName: "Color Format"
                options: Connection.colorFormatValues().map(value => ({
                    "text": Connection.colorFormatString(value),
                    "value": value
                }))
                Layout.preferredHeight: 30
                Layout.fillWidth: true
                Layout.fillHeight: false
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Button {
                    id: selectButton
                    implicitHeight: 30
                    text: Theme.icons.selectCheck
                    font.family: Theme.iconFontName
                    font.weight: Theme.iconFontWeight
                    font.pixelSize: Theme.iconFontSize
                    visible: popup.selectable
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
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
                    onClicked: popup.close();
                }
            }

            function resetMenuValues() {
                presetsComboBox.currentIndex = 0;
                cropPosMenuValue.value = "0";
                cropSizeMenuValue.value = "100%";
                resizeMenuValue.value = "100%";
                colorFormatMenuValue.value = Connection.ColorFormatARGB32_Premultiplied;
            }

            function saveMenuValues() {
                if (!popup.selected) return;
                let data = {};
                for (let menuValue of children) {
                    if (menuValue.attrKey === undefined || (!menuValue.visible && popup.visible)) continue;
                    if (menuValue instanceof DirectionsMenuValue) {
                        data[menuValue.attrKey + "Text"] = menuValue.value;
                        data[menuValue.attrKey] = menuValue.numberValues;
                    } else {
                        data[menuValue.attrKey] = menuValue.value;
                    }
                }
                popup.settings.editImage(popup.selected.key, data);
                if (popup.selectedObject)
                    imagesView.itemAtIndex(popup.selectedObject.index).removeBindings();
            }

            function loadMenuValues(data: variant, reset = true) {
                if (reset)
                    resetMenuValues();
                for (let menuValue of children) {
                    if (menuValue.attrKey === undefined || (!menuValue.visible && popup.visible)
                        || !data[menuValue.attrKey]) continue;
                    if (menuValue instanceof DirectionsMenuValue) {
                        menuValue.numberValues = data[menuValue.attrKey];
                        menuValue.value = data[menuValue.attrKey + "Text"];
                    } else {
                        menuValue.value = data[menuValue.attrKey];
                    }
                }
                presetsComboBox.updatePreset();
            }
        }
    }

    onOpened: {
        if (!selectable) {
            popup.selected = undefined;
            popup.selectedObject = undefined;
        } else if (selected) {
            Qt.callLater(() => {
                const index = imagesView.model.findIndex(obj => obj.key === popup.selected.key);
                if (index < 0) return;
                const object = imagesView.itemAtIndex(index);
                if (!object) return;
                const selected = popup.selected;
                popup.selected = undefined;
                object.select();
                modLayout.loadMenuValues(selected, false);
            });
        }
    }
    onClosed: {
        if (selectable) {
            updateSelected();
        } else {
            modLayout.saveMenuValues();
        }
    }

    function updateSelected(): void {
        if (!selected || !selectedObject) return;
        for (let menuValue of modLayout.children) {
            if (menuValue.attrKey === undefined || (!menuValue.visible && visible)) continue;
            if (menuValue instanceof DirectionsMenuValue) {
                if (menuValue.value !== selectedObject[menuValue.attrKey + "Text"]) {
                    selected[menuValue.attrKey] = menuValue.numberValues;
                    selected[menuValue.attrKey + "Text"] = menuValue.value;
                } else {
                    delete selected[menuValue.attrKey];
                    delete selected[menuValue.attrKey + "Text"];
                }
            } else {
                if (menuValue.value !== selectedObject[menuValue.attrKey]) {
                    selected[menuValue.attrKey] = menuValue.value;
                } else {
                    delete selected[menuValue.attrKey];
                }
            }
        }
        popup.selected.imageKey = Utils.createDefaultedImageKey(selected, selectedObject);
        imagesView.itemAtIndex(selectedObject.index).removeBindings();
    }

    function isSelected(key: string): bool {
        return selected !== undefined && selected.key === key;
    }
}