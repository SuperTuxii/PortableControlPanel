import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ColumnLayout {
    required property ImagesMenu imagesMenu
    property var macros: undefined

    ImageMenuValue {
        id: menuValue
        attrKey: "image"
        propName: "Image"
        imagesMenu: parent.imagesMenu
        macros: parent.macros
        Layout.preferredHeight: 30
    }
}