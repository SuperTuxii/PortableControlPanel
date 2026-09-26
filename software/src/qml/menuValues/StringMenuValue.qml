import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: layout

    property BigTextBox bigTextBox
    required property string attrKey
    property alias propName: textLabel.text
    property alias fontSize: textLabel.font.pointSize
    property alias value: textField.text

    spacing: 0
    uniformCellSizes: true
    Label {
        id: textLabel
        text: "Test"
        color: Theme.labelWhite
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        Layout.fillHeight: true
        Layout.fillWidth: true
    }
    TextField {
        id: textField
        text: ""
        color: Theme.labelWhite
        padding: 4
        horizontalAlignment: TextInput.AlignHCenter
        verticalAlignment: TextInput.AlignVCenter
        Layout.fillHeight: true
        Layout.fillWidth: true
        background: Rectangle {
            color: Theme.textFieldBackground
            border.width: Theme.textFieldBorderWidth
            border.color: Theme.textFieldBorder
            radius: Theme.textFieldRadius
        }
        onEnabledChanged: color.a = enabled ? 1.0 : 0.3

        Button {
            id: bigBoxButton
            implicitHeight: 16
            implicitWidth: 16
            text: Theme.icons.fullscreen
            font.family: Theme.iconFontName
            font.weight: Theme.iconFontWeight
            font.pixelSize: Theme.iconFontSize / 2
            visible: layout.bigTextBox
            anchors.top: parent.top
            anchors.right: parent.right
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: parent.enabled ? 1.0 : 0.3
                color: parent.down ? Theme.buttonWhiteActive : Theme.buttonWhite
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

            onClicked: {
                popup.bigTextBox.open()
                popup.bigTextBox.value = layout.value;
                layout.value = Qt.binding(() => popup.bigTextBox.value);
                const removeBindings = () => {
                    layout.value = popup.bigTextBox.value;
                    popup.bigTextBox.closed.disconnect(removeBindings);
                };
                popup.bigTextBox.closed.connect(removeBindings);
            }
        }
    }
}