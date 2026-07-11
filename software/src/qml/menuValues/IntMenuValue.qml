import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: layout

    required property string attrKey
    property alias propName: textLabel.text
    property alias fontSize: textLabel.font.pointSize
    property int min: 0
    property int max: 10
    property alias value: textField.text
    property int numberValue: min > 0 ? min : 0
    property bool valid: false
    property var preprocessor: (string) => string
    property var parser: (string) => Utils.parseIntCalc(string, min, max)

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
        text: layout.min > 0 ? layout.min : 0
        color: layout.valid ? Theme.labelWhite : Theme.labelRed
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

        Component.onCompleted: layout.revalidate();
        onTextChanged: layout.revalidate()
        onEditingFinished: layout.revalidate()

        function validate() {
            const value = layout.parser(layout.preprocessor(text));
            if (value !== undefined)
                layout.numberValue = value;
            return value !== undefined;
        }
    }

    onMinChanged: revalidate()
    onMaxChanged: revalidate()

    function revalidate(): void {
        valid = textField.validate()
    }
}