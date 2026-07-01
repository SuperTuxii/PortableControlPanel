import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: layout

    required property string attrKey
    property alias propName: textLabel.text
    property alias fontSize: textLabel.font.pointSize
    property alias options: comboBox.model
    property string value: comboBox.currentText

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
    ComboBox {
        id: comboBox
        Layout.fillHeight: true
        Layout.fillWidth: true
        background: Rectangle {
            color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
            radius: Theme.buttonRadius
            border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
            border.width: Theme.buttonBorderWidth
        }

        onCurrentTextChanged: {
            layout.value = comboBox.currentText;
        }
    }

    onValueChanged: {
        if (value !== comboBox.currentText) {
            for (let i = 0; i < comboBox.count; i++) {
                if (comboBox.valueAt(i) === value) {
                    comboBox.currentIndex = i;
                    break;
                }
            }
            if (value !== comboBox.currentText)
                comboBox.currentIndex = -1;
        }
    }
}