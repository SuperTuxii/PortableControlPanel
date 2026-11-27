import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Popup {
    id: actionsPopup

    readonly property var actions: {
        "wait": {
            "name": "Wait",
            "properties": {
                "delay": {
                    "type": "Int",
                    "config": {
                        "propName": "Delay in Seconds",
                        "min": 0,
                        "max": 86400,
                        "value": "1"
                    }
                }
            }
        }
    }
    readonly property var structure: {
        "Time": [
            "wait"
        ]
    }
    property alias value: actionsRepeater.model

    width: Math.max(Overlay.overlay.width / 4, Math.min(320, Overlay.overlay.width))
    height: Math.max(Overlay.overlay.height / 2, Math.min(640, Overlay.overlay.height))
    x: (Overlay.overlay.width / 2) - width
    y: (Overlay.overlay.height - height) / 2
    focus: true
    padding: 15

    background: Rectangle {
        color: Theme.secondaryBackground
        border.width: Theme.borderWidth
        border.color: Theme.border
        radius: Theme.borderRadius
    }

    ScrollView {
        id: actionsScroll
        anchors.fill: parent
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        clip: true
        Column {
            id: actionsColumn
            width: actionsScroll.availableWidth
            spacing: 5
            padding: 5
            Repeater {
                id: actionsRepeater
                model: []
                delegate: Rectangle {
                    id: root
                    required property int index
                    required property var model
                    height: childrenRect.height
                    width: actionsColumn.width - 2 * actionsColumn.padding
                    color: Theme.mainBackground
                    radius: Theme.buttonRadius
                    border.color: Theme.border
                    border.width: Theme.buttonBorderWidth

                    Label {
                        id: label
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: Theme.buttonRadius / 2
                        text: model.action ? actionsPopup.actions[model.action].name : ""
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Button {
                        id: upButton
                        width: font.pixelSize * 1.25
                        height: font.pixelSize * 1.25
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.margins: Theme.buttonRadius / 2
                        text: Theme.icons.arrowUp
                        font.family: Theme.iconFontName
                        font.weight: Theme.iconFontWeight
                        font.pixelSize: Theme.iconFontSize * 0.65
                        enabled: index > 0
                        contentItem: Text {
                            text: parent.text
                            font: parent.font
                            opacity: parent.enabled ? 1.0 : 0.3
                            color: parent.down ? Theme.buttonWhiteActive : Theme.buttonWhite
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                            radius: Theme.buttonRadius
                            border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                            border.width: Theme.buttonBorderWidth
                        }
                        onClicked: {
                            const itemModel = actionsRepeater.model;
                            const index = root.index;
                            const [item] = itemModel.splice(index, 1);
                            itemModel.splice(index-1, 0, item);
                        }
                    }
                    Button {
                        id: downButton
                        width: font.pixelSize * 1.25
                        height: font.pixelSize * 1.25
                        anchors.top: parent.top
                        anchors.left: upButton.right
                        anchors.margins: Theme.buttonRadius / 2
                        text: Theme.icons.arrowDown
                        font.family: Theme.iconFontName
                        font.weight: Theme.iconFontWeight
                        font.pixelSize: Theme.iconFontSize * 0.65
                        enabled: index < actionsRepeater.count-1
                        contentItem: Text {
                            text: parent.text
                            font: parent.font
                            opacity: parent.enabled ? 1.0 : 0.3
                            color: parent.down ? Theme.buttonWhiteActive : Theme.buttonWhite
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                            radius: Theme.buttonRadius
                            border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                            border.width: Theme.buttonBorderWidth
                        }
                        onClicked: {
                            const itemModel = actionsRepeater.model;
                            const index = root.index;
                            const [item] = itemModel.splice(index, 1);
                            itemModel.splice(index+1, 0, item);
                        }
                    }
                    Button {
                        id: deleteButton
                        width: font.pixelSize * 1.25
                        height: font.pixelSize * 1.25
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: Theme.buttonRadius / 2
                        text: Theme.icons.remove
                        font.family: Theme.iconFontName
                        font.weight: Theme.iconFontWeight
                        font.pixelSize: Theme.iconFontSize * 0.65
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
                        onClicked: actionsRepeater.model.splice(index, 1)
                    }
                    Column {
                        id: propsColumn
                        anchors.top: label.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        spacing: 5
                        padding: 5
                        Repeater {
                            model: []
                            delegate: Loader {
                                required property var model
                                height: 30
                                width: propsColumn.width - 2 * propsColumn.padding
                                Component.onCompleted: setSource(
                                    "menuValues/" + model.type + "MenuValue.qml",
                                    model.config
                                )
                                onLoaded: item.valueChanged.connect(() => {
                                    if (!root.model.properties) root.model.properties = {};
                                    if (model.type === "Int")
                                        root.model.properties[item.attrKey] = {
                                            "text": item.value,
                                            "value": item.numberValue
                                        }
                                    else
                                        root.model.properties[item.attrKey] = item.value;
                                })
                            }
                            Component.onCompleted: {
                                if (!root.model.action) return;
                                model = Object.entries(actionsPopup.actions[root.model.action].properties).map(([key, value]) => {
                                    let config = Object.assign({ "attrKey": key }, value.config);
                                    if (root.model.properties && key in root.model.properties) {
                                        if (value.type === "Int") {
                                            config.value = root.model.properties[key].text;
                                            config.numberValue = root.model.properties[key].value;
                                        } else {
                                            config.value = root.model.properties[key];
                                        }
                                    }
                                    return Object.assign({}, value, { "config": config });
                                });
                            }
                        }
                    }
                }
            }
            RowLayout {
                width: actionsColumn.width - 2 * actionsColumn.padding
                ComboBox {
                    id: actionComboBox
                    Layout.fillWidth: true
                    textRole: "text"
                    valueRole: "value"
                    model: []
                    popup: Menu {
                        y: actionComboBox.height

                        Component {
                            id: menuItemComponent

                            Action {
                                required property var parentMenu
                                required property string value
                                onTriggered: actionComboBox.currentValue = value
                            }
                        }

                        Component {
                            id: subMenuComponent

                            Menu {
                                id: menu
                                required property var parentMenu
                                required property string text
                                required property var value
                                title: text
                                function populateMenu() {
                                    for (let element of value) {
                                        if (Array.isArray(element) || typeof element === "object") {
                                            element.parentMenu = menu;
                                            menu.addMenu(subMenuComponent.createObject(menu.contentItem, element));
                                        } else {
                                            const elementObj = { text: actionsPopup.actions[element].name, value: element, parentMenu: actionComboBox };
                                            let action = menuItemComponent.createObject(menu.contentItem, elementObj);
                                            menu.addAction(action);
                                            actionComboBox.model.push(elementObj);
                                        }
                                    }
                                    actionComboBox.currentIndex = -1;
                                }
                                Component.onCompleted: {
                                    Qt.callLater(menu.populateMenu);
                                }
                            }
                        }

                        Component.onCompleted: {
                            for (const elementKey in actionsPopup.structure) {
                                let element = { text: elementKey, value: actionsPopup.structure[elementKey], parentMenu: actionComboBox };
                                if (Array.isArray(element.value)) {
                                    addMenu(subMenuComponent.createObject(contentItem, element));
                                } else {
                                    let action = menuItemComponent.createObject(contentItem, element);
                                    addAction(action);
                                    actionComboBox.model.push(element);
                                }
                            }
                            actionComboBox.currentIndex = -1;
                        }
                    }
                    background: Rectangle {
                        color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                        radius: Theme.buttonRadius
                        border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                        border.width: Theme.buttonBorderWidth
                    }
                }
                Button {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    Layout.alignment: Qt.AlignCenter
                    text: Theme.icons.add
                    font.family: Theme.iconFontName
                    font.weight: Theme.iconFontWeight
                    font.pixelSize: Theme.iconFontSize
                    enabled: actionComboBox.currentIndex !== -1
                    background: Rectangle {
                        color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                        radius: Theme.buttonRadius
                        border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                        border.width: Theme.buttonBorderWidth
                    }
                    onClicked: {
                        actionsRepeater.model.push({ "action": actionComboBox.currentValue });
                        actionsScroll.contentItem.contentY = actionsScroll.contentItem.contentHeight - actionsScroll.contentItem.height + 500;
                    }
                }
            }
        }
    }
}