import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: layout

    required property ImagesMenu imagesMenu
    required property string attrKey
    property alias propName: textLabel.text
    property alias fontSize: textLabel.font.pointSize
    property var value: undefined
    property var macros: undefined
    property int styleSelector: 0

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
        text: layout.value ? layout.value.key : ""
        ToolTip.text: text
        ToolTip.visible: hovered && text
        ToolTip.delay: 500
        background: Rectangle {
            color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
            radius: Theme.buttonRadius
            border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
            border.width: Theme.buttonBorderWidth
        }
        onClicked: {
            layout.imagesMenu.selected = layout.value;
            layout.imagesMenu.selectable = true;
            layout.imagesMenu.macros = Qt.binding(() => layout.macros);
            layout.imagesMenu.styleSelector = Qt.binding(() => layout.styleSelector);
            layout.imagesMenu.open();
            layout.imagesMenu.closed.connect(layout.setValue);
        }
    }

    function setValue(): void {
        layout.value = layout.imagesMenu.selected;
        layout.imagesMenu.selectable = false;
        layout.imagesMenu.macros = undefined;
        layout.imagesMenu.styleSelector = 0;
        layout.imagesMenu.closed.disconnect(layout.setValue);
    }
}