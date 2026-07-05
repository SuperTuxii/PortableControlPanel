import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    anchors.fill: parent
    parent: Overlay.overlay

    //Top-Left
    ColumnLayout{
        id: topLeftColumn
        anchors.leftMargin: 12
        anchors.left: parent.left
        anchors.top: parent.top
        width: 300
        spacing: 0
    }

    //Top-Center
    ColumnLayout{
        id: topCenterColumn
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        width: 300
        spacing: 0
    }

    //Top-Right
    ColumnLayout{
        id: topRightColumn
        anchors.rightMargin: 12
        anchors.right: parent.right
        anchors.top: parent.top
        width: 300
        spacing: 0
    }

    //Bottom-Left
    ColumnLayout{
        id: bottomLeftColumn
        anchors.leftMargin: 12
        anchors.left: parent.left
        y: root.height-height-12
        width: 300
        spacing: 0
    }

    //Bottom-Center
    ColumnLayout{
        id: bottomCenterColumn
        anchors.horizontalCenter: parent.horizontalCenter
        y: root.height-height-12
        width: 300
        spacing: 0
    }

    //Bottom-Right
    ColumnLayout{
        id: bottomRightColumn
        anchors.rightMargin: 12
        anchors.right: parent.right
        y: root.height-height-12
        width: 300
        spacing: 0
    }

    function createMessage( message, options = {} ){
        try{
            if(!message){
                throw new Error("Invalid message")
            }

            const { type, position, theme, closeOnClick, autoClose, hideProgressBar, clickAction } = options;
            const props = {};
            if (message) props.message = message;
            if (type) props.type = type;
            if (position) props.position = position;
            if (theme) props.theme = theme;
            if (closeOnClick) props.closeOnClick = closeOnClick;
            if (autoClose) props.autoClose = autoClose;
            if (hideProgressBar) props.hideProgressBar = hideProgressBar;
            if (clickAction) props.clickAction = clickAction;
            var component = Qt.createComponent("ControlPanelSoftware", "ToastifyDelegate")
            var messageContainer = component.createObject(determinePosition(position), props);
            return messageContainer;
        }
        catch(err){
            console.error(err || "Failed to create toast")
        }
    }

    function determinePosition(position){
        switch(position){
        case "top-left":
            return topLeftColumn;
        case "top-center":
            return topCenterColumn;
        case "top-right":
            return topRightColumn;
        case "bottom-left":
            return bottomLeftColumn;
        case "bottom-center":
            return bottomCenterColumn;
        case "bottom-right":
            return bottomRightColumn;
        case undefined:
            return topLeftColumn;
        default:
            console.log("Invalid position")
        }
    }
}
