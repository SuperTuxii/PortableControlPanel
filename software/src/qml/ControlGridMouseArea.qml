import QtQuick
import QtQuick.Controls


MouseArea {
    id: mouseArea
    readonly property int resizeZone: 10
    required property ControlGridQml controlGrid
    required property ControlGridBlockMenu controlGridBlockMenu
    property int drag
    property real dragX
    property real dragY
    property var block: controlGrid.findBlock(row, column)
    property bool newBlock: block === undefined
    property int row
    property int column
    property bool clickNewBlock
    property int clickRow
    property int clickColumn

    height: controlGrid.displayPanel.displayHeight
    width: controlGrid.displayPanel.displayWidth
    enabled: !controlGridBlockMenu.visible
    hoverEnabled: true
    cursorShape: Qt.ArrowCursor
    acceptedButtons: Qt.RightButton | Qt.LeftButton
    anchors.centerIn: parent

    Menu {
        id: clientBlockContextMenu

        title: "BlockMenu"
        MenuItem {
            text: mouseArea.clickNewBlock ? "Add" : "Edit"
            onTriggered: controlGridBlockMenu.open()
        }
        MenuItem {
            text: "Remove"
            enabled: !mouseArea.clickNewBlock
            onTriggered: controlGridBlockMenu.removeBlock()
        }
    }

    onPressed: (mouse) => {
        if (row < 0 || row > controlGrid.rows || column < 0 || column > controlGrid.columns) return;
        clickNewBlock = newBlock;
        clickRow = row;
        clickColumn = column;
        controlGridBlockMenu.newBlock = newBlock;
        controlGridBlockMenu.row = row;
        controlGridBlockMenu.column = column;
        if (cursorShape !== Qt.ArrowCursor) {
            dragX = mouse.x;
            dragY = mouse.y;
            controlGrid.startDrag(row, column);
        }
    }
    onReleased: (mouse) => {
        if (cursorShape !== Qt.ArrowCursor) {
            setCursorShape(mouse);
            controlGrid.endDrag();
        } else {
            if (mouse.button === Qt.RightButton) {
                clientBlockContextMenu.popup();
            } else {
                controlGridBlockMenu.open();
            }
        }
    }
    onPositionChanged: (mouse) => {
        let startX = (width - controlGrid.controlGridWidth - controlGrid.columnPad) / 2
        let startY = (height - controlGrid.controlGridHeight - controlGrid.rowPad) / 2
        row = Math.floor(((mouse.y - startY) / controlGrid.controlGridHeight) * controlGrid.rows);
        column = Math.floor(((mouse.x - startX) / controlGrid.controlGridWidth) * controlGrid.columns);
        if (!(pressedButtons & Qt.LeftButton)) {
            setCursorShape(mouse);
        } else {
            if (!newBlock && cursorShape === Qt.ArrowCursor) {
                cursorShape = Qt.ClosedHandCursor;
                dragX = mouse.x;
                dragY = mouse.y;
                controlGrid.startDrag(row, column);
            }
            if (cursorShape !== Qt.ArrowCursor) {
                let target = controlGrid.dragTarget;
                if (drag === 0) {
                    let changeX = Math.min(controlGrid.controlGridWidth - target.width - target.x, Math.max(-target.x, mouse.x - dragX));
                    let changeY = Math.min(controlGrid.controlGridHeight - target.height - target.y, Math.max(-target.y, mouse.y - dragY));
                    target.x += changeX;
                    target.y += changeY;
                    dragX += changeX;
                    dragY += changeY;
                } else {
                    let sizeN = drag % 2 === 1;
                    let sizeE = Math.floor(drag / 2) % 2 === 1;
                    let sizeS = Math.floor(drag / 4) % 2 === 1;
                    let sizeW = Math.floor(drag / 8) % 2 === 1;
                    if (sizeN) {
                        let changeY = Math.min(target.height - 0.5 * (controlGrid.controlGridHeight / controlGrid.rows), Math.max(-target.y, mouse.y - dragY));
                        target.y += changeY;
                        target.height -= changeY;
                        dragY += changeY;
                    }
                    if (sizeE) {
                        let changeX = Math.min(controlGrid.controlGridWidth - target.width - target.x, Math.max(0.5 * (controlGrid.controlGridWidth / controlGrid.columns) - target.width, mouse.x - dragX));
                        target.width += changeX;
                        dragX += changeX;
                    }
                    if (sizeS) {
                        let changeY = Math.min(controlGrid.controlGridHeight - target.height - target.y, Math.max(0.5 * (controlGrid.controlGridHeight / controlGrid.rows) - target.height, mouse.y - dragY));
                        target.height += changeY;
                        dragY += changeY;
                    }
                    if (sizeW) {
                        let changeX = Math.min(target.width - 0.5 * (controlGrid.controlGridWidth / controlGrid.columns), Math.max(-target.x, mouse.x - dragX));
                        target.x += changeX;
                        target.width -= changeX;
                        dragX += changeX;
                    }
                }
                controlGrid.applyDragTransforms();
            }
        }
    }

    function setCursorShape(mouse): void {
        let startX = (width - controlGrid.controlGridWidth - controlGrid.columnPad) / 2
        let startY = (height - controlGrid.controlGridHeight - controlGrid.rowPad) / 2
        if (!newBlock && mouse.x >= startX && mouse.x <= (startX + controlGrid.controlGridWidth)
            && mouse.y >= startY && mouse.y <= (startY + controlGrid.controlGridHeight)) {
            let blockWidth = controlGrid.controlGridWidth / controlGrid.columns;
            let blockHeight = controlGrid.controlGridHeight / controlGrid.rows;
            let relativeX = ((mouse.x - startX + 0.5*blockWidth) % blockWidth) - 0.5*blockWidth;
            let relativeY = ((mouse.y - startY + 0.5*blockHeight) % blockHeight) - 0.5*blockHeight;
            let sizeN = relativeY >= 0 && relativeY < resizeZone && block.row === row;
            let sizeE = relativeX < 0 && relativeX >= -resizeZone && block.column + block.columnSpan - 1 === column;
            let sizeS = relativeY < 0 && relativeY >= -resizeZone && block.row + block.rowSpan - 1 === row;
            let sizeW = relativeX >= 0 && relativeX < resizeZone && block.column === column;
            if ((sizeN && sizeE) || (sizeS && sizeW)) {
                cursorShape = Qt.SizeBDiagCursor;
            } else if ((sizeN && sizeW) || (sizeS && sizeE)) {
                cursorShape = Qt.SizeFDiagCursor;
            } else if (sizeN || sizeS) {
                cursorShape = Qt.SizeVerCursor;
            } else if (sizeE || sizeW) {
                cursorShape = Qt.SizeHorCursor;
            } else {
                cursorShape = Qt.ArrowCursor;
            }
            drag = sizeN + sizeE * 2 + sizeS * 4 + sizeW * 8;
        } else {
            cursorShape = Qt.ArrowCursor;
        }
    }
}