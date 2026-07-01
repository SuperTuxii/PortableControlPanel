import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: layout

    required property ColorPicker colorPicker
    required property string attrKey
    property alias propName: textLabel.text
    property alias fontSize: textLabel.font.pointSize
    property color value: "#FFFFFF"

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
    Button {
        Layout.fillHeight: true
        Layout.fillWidth: true
        contentItem: Rectangle {
            color: layout.value
            radius: 5
            border.width: 1
            border.color: "black"
            anchors.fill: parent
            anchors.margins: 5
            anchors.leftMargin: 30
            anchors.rightMargin: 30
        }
        background: Rectangle {
            color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
            radius: Theme.buttonRadius
            border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
            border.width: Theme.buttonBorderWidth
        }
        onClicked: {
            layout.colorPicker.setColor(layout.value);
            layout.value = Qt.binding(function() { return layout.colorPicker.selectedColor })
            layout.colorPicker.open()
            layout.colorPicker.closed.connect(layout.removeColorBinding)
        }
    }

    function removeColorBinding(): void {
        layout.value = layout.colorPicker.selectedColor
        layout.colorPicker.closed.disconnect(layout.removeColorBinding)
    }
}