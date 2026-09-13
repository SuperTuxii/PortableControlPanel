import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ColumnLayout {
    Label {
        text: "Text"
        font.pointSize: 13
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        Layout.fillWidth: true
        Layout.topMargin: 5
        Layout.bottomMargin: 5
    }
    ScrollView {
        readonly property string attrKey: "text"
        property alias value: textArea.text
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.bottomMargin: 5

        TextArea {
            id: textArea
            text: ""
            color: Theme.labelWhite
            background: Rectangle {
                color: Theme.textFieldBackground
                border.width: Theme.textFieldBorderWidth
                border.color: Theme.textFieldBorder
                radius: Theme.textFieldRadius
            }
        }
    }
}
