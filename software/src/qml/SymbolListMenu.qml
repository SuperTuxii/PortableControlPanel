import QtQuick
import QtQuick.Controls

Popup {
    id: popup
    signal selected(symbolName: string)
    property string searchString: ""
    property var symbolInformation: JSON.parse(Connection.loadSymbolConfig())

    width: Math.max(Overlay.overlay.width / 4, Math.min(320, Overlay.overlay.width))
    height: Math.max(Overlay.overlay.height / 4, Math.min(320, Overlay.overlay.height))
    focus: false
    closePolicy: Popup.CloseOnPressOutside

    background: Rectangle {
        color: Theme.secondaryBackground
        border.width: Theme.borderWidth
        border.color: Theme.border
        radius: Theme.borderRadius
    }

    contentItem: Item {
        GridView {
            id: symbolView
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(model.length, Math.floor(parent.width / cellWidth)) * cellWidth
            height: parent.height
            cellWidth: 65
            cellHeight: 65
            clip: true
            model: {
                const symbolNames = new Map();
                if (!symbolInformation || !symbolInformation.icons) return;
                for (const icon of symbolInformation.icons) {
                    if (icon.unsupported_families.includes("Material Symbols Rounded")) continue;
                    for (const tag of icon.tags) {
                        if (tag.includes(popup.searchString)) {
                            symbolNames.set(icon.name, icon.popularity);
                            break;
                        }
                    }
                }
                return Array.from(symbolNames).sort((entry1, entry2) => entry2[1] - entry1[1]).map(entry => entry[0]);
            }
            delegate: Item {
                id: root
                required property string modelData
                width: symbolView.cellWidth
                height: symbolView.cellHeight

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 5
                    color: Qt.darker(Theme.mainBackground, tap.pressed ? Theme.buttonBackgroundDarker : hover.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
                    border.width: Theme.sidebarButtonBorderWidth
                    border.color: Theme.border
                    radius: Theme.sidebarButtonRadius
                    ToolTip.text: modelData.split("_").map(word => word.charAt(0).toUpperCase() + word.substring(1)).join(" ");
                    ToolTip.visible: hover.hovered
                    ToolTip.delay: 500

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        font.family: Theme.iconFontName
                        font.weight: Theme.iconFontWeight
                        font.pixelSize: Theme.sidebarIconSize
                        opacity: parent.enabled ? 1.0 : 0.3
                        color: parent.down ? Theme.buttonWhiteActive : Theme.buttonWhite
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    TapHandler {
                        id: tap
                        onTapped: popup.selected(modelData)
                    }

                    HoverHandler {
                        id: hover
                    }
                }
            }
            ScrollBar.vertical: ScrollBar {}
            WheelHandler {
                acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                onWheel: (event) => {
                    const newY = symbolView.contentY - (event.angleDelta.y / 120) * symbolView.cellHeight;
                    symbolView.contentY = Math.max(0, Math.min(newY, symbolView.contentHeight - symbolView.height));
                }
            }
        }
    }

    onClosed: searchString = ""
}