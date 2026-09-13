import QtQuick
import QtQuick.Controls

Popup {
    id: popup
    property alias value: textArea.text

    x: Overlay.overlay.width / 2
    y: (Overlay.overlay.height - height) / 2
    width: Math.max(Overlay.overlay.width / 4, Math.min(320, Overlay.overlay.width))
    height: Math.max(Overlay.overlay.height / 2, Math.min(640, Overlay.overlay.height))
    focus: true
    modal: true
    padding: 20

    ScrollView {
        anchors.fill: parent

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