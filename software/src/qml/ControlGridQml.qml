import QtCore
import QtQuick
import LvglSimulator

ControlGrid {
    id: controlGrid
    required property LvglDisplay displayPanel
    required property Settings settings
    property string layoutName: "test"
    property var layoutData: settings.loadLayout(layoutName)
    property var dragTarget: {
        "baseX": 0, "baseY": 0, "x": 0, "y": 0,
        "baseWidth": 0, "baseHeight": 0, "width": 0, "height": 0,
        "row": 0, "column": 0, "rowSpan": 0, "columnSpan": 0, "index": 0
    }

    Component.onCompleted: {
        displayPanel.transferRenderer(controlGrid);
        displayPanel.displaySizeRefreshed.connect(loadLayout);
        Connection.tryConnect();
        loadLayout();
    }
    onLayoutDataChanged: loadLayout

    function loadLayout(): void {
        setLayout(rows, columns);
        Connection.setLayout(rows, columns);
        settings.loadBlocks(layoutName, addBlock);
    }

    function findBlock(row: int, column: int): variant {
        if (row < 0 || row >= rows || column < 0 || column >= columns)
            return;
        for (let i in layoutData.blocks) {
            let block = layoutData.blocks[i];
            if ((row - block.row) >= 0 && (row - block.row) < block.rowSpan &&
                (column - block.column) >= 0 && (column - block.column) < block.columnSpan) {
                return block;
            }
        }
    }

    function addBlock(data): boolean {
        let index = (data.row * columns) + data.column;
        let index2 = index + (data.columnSpan-1) + ((data.rowSpan-1) * columns);
        if (!addWidget(data.type, index, index2))
            return false;

        let sizePosData = { index: index };
        insertCoordsData(sizePosData);
        const macros = {};
        Utils.buildMacros(
            macros, data.style,
            sizePosData.width, sizePosData.height,
            data.row, data.column,
            data.rowSpan, data.columnSpan
        );
        if (Utils.refreshStyleData(macros, data.style))
            settings.editBlock(layoutName, data.row, data.column, { style: data.style });

        for (const styleSelector in data.style) {
            setStyle(index, 0, styleSelector, data.style[styleSelector]);
        }
        Connection.addWidget(data.type, index, index2, data.style);
        return true;
    }

    function startDrag(row, column): void {
        let block = findBlock(row, column);
        dragTarget.index = (columns * block.row) + block.column
        dragTarget.row = block.row;
        dragTarget.column = block.column;
        dragTarget.rowSpan = block.rowSpan;
        dragTarget.columnSpan = block.columnSpan;
        insertCoordsData(dragTarget);
    }

    function applyDragTransforms(): void {
        controlGrid.setStyle(dragTarget.index, 0, Connection.PartMain, [
            { attrKey: Connection.TranslateX, value: [dragTarget.x - dragTarget.baseX + 0.5 * (dragTarget.width - dragTarget.baseWidth), dragTarget.y - dragTarget.baseY + 0.5 * (dragTarget.height - dragTarget.baseHeight)] },
            { attrKey: Connection.TransformWidth, value: [0.5 * (dragTarget.width - dragTarget.baseWidth), 0.5 * (dragTarget.height - dragTarget.baseHeight)] }
        ]);
        let row = Math.min(rows - 1, Math.max(0, Math.round((dragTarget.y / controlGridHeight) * rows)));
        let column = Math.min(columns - 1, Math.max(0, Math.round((dragTarget.x / controlGridWidth) * columns)));
        let rowSpan = Math.min(rows - row, Math.max(1, Math.round((dragTarget.height / controlGridHeight) * rows)));
        let columnSpan = Math.min(columns - column, Math.max(1, Math.round((dragTarget.width / controlGridWidth) * columns)));
        for (let i = 0; i < rowSpan * columnSpan; i++) {
            let block = findBlock(row + Math.floor(i / columnSpan), column + i % columnSpan);
            if (block !== undefined && (columns * block.row) + block.column !== dragTarget.index)
                return;
        }
        dragTarget.row = row;
        dragTarget.column = column;
        dragTarget.rowSpan = rowSpan;
        dragTarget.columnSpan = columnSpan;
    }

    function endDrag(): void {
        let block = findBlock(Math.floor(dragTarget.index / columns), dragTarget.index % columns);
        let toIndex = (dragTarget.row * columns) + dragTarget.column;
        if (toIndex !== dragTarget.index)
            controlGrid.move(dragTarget.index, toIndex);
        if (dragTarget.rowSpan !== block.rowSpan || dragTarget.columnSpan !== block.columnSpan)
            controlGrid.changeSize(toIndex, toIndex + ((dragTarget.rowSpan - 1) * columns) + (dragTarget.columnSpan - 1));
        let data = {
            row: dragTarget.row,
            column: dragTarget.column,
            rowSpan: dragTarget.rowSpan,
            columnSpan: dragTarget.columnSpan,
        }
        let sizePosData = { index: toIndex };
        insertCoordsData(sizePosData);
        const macros = {};
        Utils.buildMacros(
            macros, block.style,
            sizePosData.width, sizePosData.height,
            dragTarget.row, dragTarget.row,
            dragTarget.rowSpan, dragTarget.columnSpan
        );
        if (Utils.refreshStyleData(macros, block.style)) {
            data.style = block.style;
            remove((data.row * columns) + data.column, 0);
            addWidget(block.type, toIndex, toIndex + ((dragTarget.rowSpan - 1) * columns) + (dragTarget.columnSpan - 1));
            for (const styleSelector in data.style) {
                setStyle(toIndex, 0, styleSelector, data.style[styleSelector]);
            }
            Connection.remove(dragTarget.index, 0);
            Connection.addWidget(block.type, toIndex, toIndex + ((dragTarget.rowSpan - 1) * columns) + (dragTarget.columnSpan - 1), data.style);
        } else {
            if (0 in block.style)
                controlGrid.setStyle(toIndex, 0, Connection.PartMain, [
                    {
                        attrKey: Connection.TranslateX,
                        value: (block.style[0].find((style) => style.attrKey === Connection.TranslateX) ?? {value: [0, 0]}).value
                    },
                    {
                        attrKey: Connection.TransformWidth,
                        value: (block.style[0].find((style) => style.attrKey === Connection.TransformWidth) ?? {value: [0, 0]}).value
                    }
                ]);
            else
                controlGrid.setStyle(toIndex, 0, Connection.PartMain, [
                    {attrKey: Connection.TranslateX, value: [0, 0]},
                    {attrKey: Connection.TransformWidth, value: [0, 0]}
                ]);
            if (toIndex !== dragTarget.index)
                Connection.move(dragTarget.index, toIndex);
            if (dragTarget.rowSpan !== block.rowSpan || dragTarget.columnSpan !== block.columnSpan)
                Connection.changeSize(toIndex, toIndex + ((dragTarget.rowSpan - 1) * columns) + (dragTarget.columnSpan - 1));
        }
        settings.editBlock(layoutName, block.row, block.column, data);
    }
}