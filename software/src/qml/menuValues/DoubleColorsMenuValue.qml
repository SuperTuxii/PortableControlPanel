import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: layout

    required property ColorPicker colorPicker
    required property string attrKey
    required property string attrKey2
    property int activeValue: -1
    property alias propName: textLabel.text
    property alias fontSize: textLabel.font.pointSize
    property real colorFactor: 1.5
    property color value: "#FFFFFF"
    property color value2: Qt.darker(value, colorFactor)

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
    RowLayout {
        spacing: 5
        uniformCellSizes: true
        Layout.fillHeight: true
        Layout.fillWidth: true
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
                anchors.leftMargin: 10
                anchors.rightMargin: 10
            }
            background: Rectangle {
                color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                radius: Theme.buttonRadius
                border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                border.width: Theme.buttonBorderWidth
            }
            onClicked: {
                layout.colorPicker.setColor(layout.value);
                layout.value = Qt.binding(function () {
                    return layout.colorPicker.selectedColor
                });
                layout.activeValue = 0;
                layout.colorPicker.open();
                layout.colorPicker.closed.connect(layout.removeColorBinding);
            }
        }
        Button {
            Layout.fillHeight: true
            Layout.fillWidth: true
            contentItem: Rectangle {
                color: layout.value2
                radius: 5
                border.width: 1
                border.color: "black"
                anchors.fill: parent
                anchors.margins: 5
                anchors.leftMargin: 10
                anchors.rightMargin: 10
            }
            background: Rectangle {
                color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                radius: Theme.buttonRadius
                border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
                border.width: Theme.buttonBorderWidth
            }
            onClicked: {
                layout.colorPicker.setColor(layout.value2);
                layout.value2 = Qt.binding(function () {
                    return layout.colorPicker.selectedColor
                });
                layout.activeValue = 1;
                layout.colorPicker.open();
                layout.colorPicker.closed.connect(layout.removeColorBinding);
            }
        }
    }

    function removeColorBinding(): void {
        if (layout.activeValue === 0) {
            layout.value = layout.colorPicker.selectedColor
        } else if (layout.activeValue === 1) {
            layout.value2 = layout.colorPicker.selectedColor
        }
        layout.colorPicker.closed.disconnect(layout.removeColorBinding)
    }
}