import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: layout

    required property string attrKey
    property alias propName: textLabel.text
    property alias fontSize: textLabel.font.pointSize
    property alias min: spinBox.from
    property alias max: spinBox.to
    property alias value: spinBox.value

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
    SpinBox {
        id: spinBox
        from: 0
        to: 10
        value: 0
        editable: true
        Layout.fillHeight: true
        Layout.fillWidth: true
        background: Rectangle {
            color: Qt.darker(Theme.mainBackground, parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
            radius: Theme.buttonRadius
            border.color: Qt.darker(Theme.border, parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
            border.width: Theme.buttonBorderWidth
        }
    }
}