import QtQuick
import LvglSimulator

Item {
    signal displaySizeRefreshed()
    property alias name: lvglRenderer.name
    property alias displayWidth: lvglRenderer.width
    property alias displayHeight: lvglRenderer.height
    property alias tickPeriod: lvglRenderer.tickPeriod
    property alias displayBufferRatio: lvglRenderer.displayBufferRatio
    property rect imageClipRect: Qt.rect(0, 0, displayWidth, displayHeight)
    implicitWidth: displayWidth
    implicitHeight: displayHeight
    clip: true


    Item {
        anchors.fill: parent
        focus: true
        Keys.onPressed: event => lvglRenderer.backend.onKeyEvent(event.key, true)
        Keys.onReleased: event => lvglRenderer.backend.onKeyEvent(event.key, false)
    }

    // `image.source` has been binded to `swap`, so updating `swap` re-runs `requestPixmap`.
    //  Disable caching to call into `LvglImageProvider::requestPixmap` each time.
    Image {
        id: image
        property int swap: 0
        x: -parent.imageClipRect.x * (parent.implicitWidth / parent.imageClipRect.width)
        y: -parent.imageClipRect.y * (parent.implicitHeight / parent.imageClipRect.height)
        width: parent.displayWidth * (parent.implicitWidth / parent.imageClipRect.width)
        height: parent.displayHeight * (parent.implicitHeight / parent.imageClipRect.height)
        source: "image://" + lvglRenderer.name + "/buf" + swap
        cache: false

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            Component.onCompleted: lvglRenderer.setMouseArea(mouseArea)
        }
    }

    Timer {
        id: timer
        interval: lvglRenderer.tickPeriod
        running: true
        repeat: true
        onTriggered: image.swap = (image.swap ? 0 : 1)
    }

    LvglRenderer {
        id: lvglRenderer
    }

    Component.onCompleted: lvglRenderer.displaySizeRefreshed.connect(displaySizeRefreshed)

    function transferRenderer(object) {
        object.lvglRenderer = lvglRenderer;
    }

    function changeDisplaySize(width: int, height: int): void {
        lvglRenderer.setDisplaySize(width, height);
    }
}
