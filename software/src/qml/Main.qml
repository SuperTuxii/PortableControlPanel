import QtQuick
import QtQuick.Layouts

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("PortableControlPanel Software")
    color: Theme.mainBackground
    minimumWidth: mainLayout.implicitWidth
    minimumHeight: mainLayout.implicitHeight

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: displayPanel.width
            Layout.minimumHeight: displayPanel.height
            color: Theme.mainBackground

            Rectangle {
                id: displayPanel
                property int displayHeight: 480
                property int displayWidth: 800
                property real displayScale: 1

                anchors.centerIn: parent
                width: (displayWidth * displayScale) + (2 * Theme.displayBorderRadius)
                height: (displayHeight * displayScale) + (2 * Theme.displayBorderRadius)
                radius: Theme.displayBorderRadius
                border.width: Theme.displayBorderRadius
                border.pixelAligned: true
                color: Theme.displayBorder

                ControlGrid {
                    width: displayPanel.displayWidth
                    height: displayPanel.displayHeight
                }
            }
        }
    }
}