import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Popup {
    id: popup

    readonly property real gradientPickerSize: 250
    readonly property real cursorRadius: 5
    readonly property real cursorWidth: 2
    property color selectedColor

    width: 320
    height: 480
    focus: true
    padding: 30

    background: Rectangle {
        color: Theme.secondaryBackground
        border.width: Theme.borderWidth
        border.color: Theme.border
        radius: Theme.borderRadius
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: 25
        Item {
            width: popup.gradientPickerSize
            height: popup.gradientPickerSize
            Layout.alignment: Qt.AlignHCenter
            Rectangle {
                x: 0
                y: 0
                anchors.fill: parent
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop {
                        position: 0.0
                        color: "#FFFFFF"
                    }
                    GradientStop {
                        position: 1.0
                        color: Qt.hsva(popup.selectedColor.hsvHue, 1.0, 1.0, 1.0)
                    }
                }
            }
            Rectangle {
                x: 0
                y: 0
                anchors.fill: parent
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop {
                        position: 0.0
                        color: "#00000000"
                    }
                    GradientStop {
                        position: 1.0
                        color: "#FF000000"
                    }
                }
            }
            Rectangle {
                id: cursor
                x: -popup.cursorRadius
                y: -popup.cursorRadius
                width: popup.cursorRadius*2
                height: popup.cursorRadius*2
                radius: popup.cursorRadius
                border.color: Theme.secondaryBorder
                border.width: popup.cursorWidth
                color: "transparent"
            }
            MouseArea {
                x: 0
                y: 0
                anchors.fill: parent
                function handleMouse(mouse: MouseEvent) {
                    if (mouse.buttons & Qt.LeftButton) {
                        cursor.x = Math.max(0, Math.min(popup.gradientPickerSize, mouse.x)) - popup.cursorRadius;
                        cursor.y = Math.max(0, Math.min(popup.gradientPickerSize, mouse.y)) - popup.cursorRadius;
                        popup.refreshColor();
                    }
                }
                onPositionChanged: (mouse) => { handleMouse(mouse) }
                onPressed: (mouse) => { handleMouse(mouse) }
            }
        }
        Slider {
            id: hueSlider
            from: 0.0
            value: popup.selectedColor.hsvHue
            to: 1.0
            Layout.fillWidth: true

            background: Rectangle {
                height: 18
                radius: 9
                color: "transparent"
                border.width: Theme.borderWidth
                border.color: Theme.secondaryBorder
                Rectangle {
                    implicitWidth: parent.width - 8
                    implicitHeight: parent.height - 8
                    radius: 5
                    anchors.centerIn: parent
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0;  color: "#FF0000" }
                        GradientStop { position: 0.15; color: "#FFFF00" }
                        GradientStop { position: 0.24; color: "#00FF00" }
                        GradientStop { position: 0.5;  color: "#00FFFF" }
                        GradientStop { position: 0.67; color: "#0000FF" }
                        GradientStop { position: 0.84; color: "#FF00FF" }
                        GradientStop { position: 1.0;  color: "#FF0000" }
                    }
                }
            }
            handle: Rectangle {
                x: parent.visualPosition * (parent.availableWidth - width - 4) + 2
                y: 2
                implicitHeight: 14
                implicitWidth: 14
                radius: 7
                color: "transparent"
                border.width: Theme.borderWidth
                border.color: "#505050"
            }

            onMoved: popup.refreshColor()
        }
        Slider {
            id: alphaSlider
            from: 0.0
            value: popup.selectedColor.a
            to: 1.0
            Layout.fillWidth: true

            background: Rectangle {
                height: 18
                radius: 9
                color: "transparent"
                border.width: Theme.borderWidth
                border.color: Theme.secondaryBorder
                Rectangle {
                    implicitWidth: parent.width - 8
                    implicitHeight: parent.height - 8
                    radius: 5
                    anchors.centerIn: parent
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop {
                            position: 1.0
                            color: Qt.rgba(popup.selectedColor.r, popup.selectedColor.g, popup.selectedColor.b, 1.0)
                        }
                        GradientStop {
                            position: 0.0
                            color: Qt.rgba(popup.selectedColor.r, popup.selectedColor.g, popup.selectedColor.b, 0.0)
                        }
                    }
                }
            }
            handle: Rectangle {
                x: parent.visualPosition * (parent.availableWidth - width - 4) + 2
                y: 2
                implicitHeight: 14
                implicitWidth: 14
                radius: 7
                color: "transparent"
                border.width: Theme.borderWidth
                border.color: "#505050"
            }

            onMoved: popup.refreshColor()
        }
        TextField {
            text: (popup.selectedColor.a === 1 ? popup.selectedColor.toString() : "#" + popup.selectedColor.toString().slice(3, 9) + popup.selectedColor.toString().slice(1, 3)).toUpperCase()
            placeholderText: "HEX"
            horizontalAlignment: TextInput.AlignHCenter
            verticalAlignment: TextInput.AlignVCenter
            font.pointSize: 12
            Layout.fillWidth: true
            Layout.leftMargin: 25
            Layout.rightMargin: 25
            background: Rectangle {
                implicitHeight: 45
                color: Theme.textFieldBackground
                border.width: Theme.textFieldBorderWidth
                border.color: Theme.textFieldBorder
                radius: Theme.textFieldRadius
            }

            onEditingFinished: {
                if (text.length === 0) { return; }
                popup.setColor(text.length > 7 ? "#" + text.slice(7, 9) + text.slice(1, 7) : text);
                text = Qt.binding(function () { return (popup.selectedColor.a === 1 ? popup.selectedColor.toString() : "#" + popup.selectedColor.toString().slice(3, 9) + popup.selectedColor.toString().slice(1, 3)).toUpperCase() });
            }
        }
    }

    function setColor(selectColor: color): void {
        if (selectColor.hsvHue < 0) { selectColor.hsvHue = 0; }
        cursor.x = selectColor.hsvSaturation * popup.gradientPickerSize - popup.cursorRadius
        cursor.y = (1 - selectColor.hsvValue) * popup.gradientPickerSize - popup.cursorRadius
        hueSlider.value = selectColor.hsvHue;
        alphaSlider.value = selectColor.a;
        refreshColor();
    }

    function refreshColor(): void {
        popup.selectedColor.hsvSaturation = (cursor.x + popup.cursorRadius) / popup.gradientPickerSize;
        popup.selectedColor.hsvValue = 1 - (cursor.y + popup.cursorRadius) / popup.gradientPickerSize;
        popup.selectedColor.hsvHue = hueSlider.value;
        popup.selectedColor.a = alphaSlider.value;
    }
}