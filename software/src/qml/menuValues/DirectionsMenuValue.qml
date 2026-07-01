import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: layout

    required property string attrKey
    property alias propName: textLabel.text
    property alias fontSize: textLabel.font.pointSize
    property int directions: 4
    property int min: 0
    property int max: 10
    property alias value: textField.text
    property list<int> numberValues: []
    property bool valid: false

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
        horizontalAlignment: TextInput.AlignHCenter
        verticalAlignment: TextInput.AlignVCenter
        Layout.fillHeight: true
        Layout.fillWidth: true
        background: Rectangle {
            implicitHeight: 45
            color: Theme.textFieldBackground
            border.width: Theme.textFieldBorderWidth
            border.color: Theme.textFieldBorder
            radius: Theme.textFieldRadius
        }

        Component.onCompleted: layout.valid = validate()
        onTextChanged: layout.valid = validate()
        onEditingFinished: {
            tryCorrect();
            layout.valid = validate();
        }


        function tryCorrect() {
            if (text.length === 0) {
                text = layout.min > 0 ? layout.min : 0;
            } else {
                if (!(/^[\s\d;-]*$/).test(text))
                    return;
                let values = text.split(";");
                if (values.length !== 1 && values.length !== layout.directions && !(layout.directions === 4 && values.length === 2))
                    return;
                for (const i in values) {
                    const numberValue = parseInt(values[i]);
                    if (isNaN(numberValue))
                        values[i] = layout.min > 0 ? layout.min : 0;
                    if (numberValue < layout.min)
                        values[i] = layout.min;
                    if (numberValue > layout.max)
                        values[i] = layout.max;
                }
                text = values.join(";");
            }
        }

        function validate() {
            if (text.length !== 0) {
                if (!(/^[\s\d;-]*$/).test(text))
                    return false;
                let values = text.split(";");
                if (values.length !== 1 && values.length !== layout.directions && !(layout.directions === 4 && values.length === 2))
                    return false;
                for (const value of values) {
                    const numberValue = parseInt(value);
                    if (isNaN(numberValue) || numberValue < layout.min || numberValue > layout.max)
                        return false;
                }
                layout.numberValues = values;
                return true;
            }
            return false;
        }
    }
}