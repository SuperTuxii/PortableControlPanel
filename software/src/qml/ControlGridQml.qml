import QtCore
import QtQuick
import LvglSimulator

ControlGrid {
    id: controlGrid
    required property LvglDisplay displayPanel
    required property Settings settings
    property string layoutName: ""
    property var layoutData: settings.loadLayout(layoutName)
    property var dragTarget: {
        "baseX": 0, "baseY": 0, "x": 0, "y": 0,
        "baseWidth": 0, "baseHeight": 0, "width": 0, "height": 0,
        "row": 0, "column": 0, "rowSpan": 0, "columnSpan": 0, "index": 0
    }

    Component.onCompleted: {
        displayPanel.transferRenderer(controlGrid);
        displayPanel.displaySizeRefreshed.connect(loadLayout);
        loadLayout();
        Connection.tryConnect();
        layoutNameChanged.connect(loadLayout);
    }

    function loadLayout(): void {
        if (!settings.layoutExists(layoutName)) return;
        layoutData = Qt.binding(() => settings.loadLayout(layoutName));
        removeScreenStyles();
        setLayout(layoutData.rows, layoutData.columns);
        outerPad = layoutData.outerPad;
        rowPad = layoutData.rowPad;
        columnPad = layoutData.columnPad;
        Connection.removeScreenStyles();
        Connection.setLayout(layoutData.rows, layoutData.columns);
        Connection.setOuterPad(layoutData.outerPad);
        Connection.setRowPad(layoutData.rowPad);
        Connection.setColumnPad(layoutData.columnPad);
        updateScreenStyleMacros();
        updateImages();
        if (layoutData.style) {
            for (const styleSelector in layoutData.style) {
                setScreenStyle(styleSelector, layoutData.style[styleSelector]);
            }
            Connection.setScreenStyle(layoutData.style);
        }
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

    function updateImages(row = -1, column = -1, data = undefined): void {
        let images = new Set();
        if (layoutData.style) {
            for (const styleSelector in layoutData.style) {
                for (const styleElement of layoutData.style[styleSelector]) {
                    if (styleElement.attrKey === Connection.BackgroundImageIndex
                        && styleElement.value && styleElement.value.imageKey && styleElement.value.imageKey.length > 1)
                        images.add(styleElement.value.imageKey);
                }
            }
        }
        for (const block of layoutData.blocks) {
            if (row - block.row >= 0 && row - block.row < block.rowSpan
                && column - block.column >= 0 && column - block.column < block.columnSpan)
                continue;
            for (const styleSelector in block.style) {
                for (const styleElement of block.style[styleSelector]) {
                    if (styleElement.attrKey === Connection.BackgroundImageIndex && styleElement.value.imageKey.length > 1)
                        images.add(styleElement.value.imageKey);
                }
            }
            for (const subWidget of block.subWidgets) {
                if (subWidget.type === "Image" && subWidget.image && subWidget.image.imageKey.length > 1)
                    images.add(subWidget.image.imageKey);

                for (const styleSelector in subWidget.style) {
                    for (const styleElement of subWidget.style[styleSelector]) {
                        if (styleElement.attrKey === Connection.BackgroundImageIndex && styleElement.value.imageKey.length > 1)
                            images.add(styleElement.value.imageKey);
                    }
                }
            }
        }
        if (data) {
            for (const styleSelector in data.style) {
                for (const styleElement of data.style[styleSelector]) {
                    if (styleElement.attrKey === Connection.BackgroundImageIndex && styleElement.value.imageKey.length > 1)
                        images.add(styleElement.value.imageKey);
                }
            }
            for (const subWidget of data.subWidgets) {
                if (subWidget.type === "Image" && subWidget.image && subWidget.image.imageKey.length > 1)
                    images.add(subWidget.image.imageKey);
                for (const styleSelector in subWidget.style) {
                    for (const styleElement of subWidget.style[styleSelector]) {
                        if (styleElement.attrKey === Connection.BackgroundImageIndex && styleElement.value.imageKey.length > 1)
                            images.add(styleElement.value.imageKey);
                    }
                }
            }
        }
        loadImages(Array.from(images));
        Connection.loadImages(Array.from(images));
    }
    function updateScreenStyleMacros(): void {
        if (!layoutData.style) return;
        const macros = {};
        Utils.buildScreenMacros(
            macros,
            layoutData.style,
            displayPanel.displayWidth,
            displayPanel.displayHeight,
            rows,
            columns,
            outerPad,
            rowPad,
            columnPad
        );
        if (Utils.refreshStyleData(macros, layoutData.style))
            settings.editLayout(layoutName, { style: layoutData.style });
    }
    function updateMacros(index: int, data, onMainChanged, onSubWidgetsChanged): void {
        let sizePosData = { index: index };
        controlGrid.insertCoordsData(sizePosData);
        const mainMacros = {};
        Utils.buildMacros(
            mainMacros, data.style,
            sizePosData.width, sizePosData.height,
            data.row, data.column,
            data.rowSpan, data.columnSpan
        );
        if (Utils.refreshStyleData(mainMacros, data.style))
            onMainChanged();
        let changed = false;
        for (let i = 0; i < data.subWidgets.length; i++) {
            const subWidget = data.subWidgets[i];
            let subSizePosData = { index: index, subIndex: i+1 };
            controlGrid.insertCoordsData(subSizePosData);
            const subMacros = {};
            Utils.buildMacros(
                subMacros, subWidget.style,
                subSizePosData.width, subSizePosData.height,
                data.row, data.column,
                data.rowSpan, data.columnSpan
            );
            subMacros.parent = mainMacros;
            if (Utils.refreshSubWidget(subMacros, subWidget)) {
                if (subWidget.type === "Image" && (!subWidget.image || !subWidget.image.imageKey || subWidget.image.imageKey.length <= 1)) continue;
                const subIndex = controlGrid.subWidget(subWidget.type, index, i+1, subWidget);
                if (subIndex !== i+1) return;
                changed = true;
            }
        }
        if (changed)
            onSubWidgetsChanged();
    }
    function addBlock(data): boolean {
        let index = (data.row * columns) + data.column;
        let index2 = index + (data.columnSpan-1) + ((data.rowSpan-1) * columns);
        if (!addWidget(data.type, index, index2))
            return false;
        for (let i = 0; i < data.subWidgets.length; i++) {
            const subWidget = data.subWidgets[i];
            if (subWidget.type === "Image" && (!subWidget.image || !subWidget.image.imageKey || subWidget.image.imageKey.length <= 1)) continue;
            const subIndex = controlGrid.subWidget(subWidget.type, index, 0, subWidget);
            if (subIndex !== i+1) return false;
        }

        updateMacros(
            index, data,
            () => settings.editBlock(layoutName, data.row, data.column, { style: data.style }),
            () => settings.editBlock(layoutName, data.row, data.column, { subWidgets: data.subWidgets })
        );

        for (const styleSelector in data.style) {
            setStyle(index, 0, styleSelector, data.style[styleSelector]);
        }
        Connection.addWidget(data.type, index, index2, data.style);
        for (let i = 0; i < data.subWidgets.length; i++) {
            const subWidget = data.subWidgets[i];
            if (subWidget.type === "Image" && (!subWidget.image || !subWidget.image.imageKey || subWidget.image.imageKey.length <= 1)) continue;
            const subIndex = i + 1;
            for (const styleSelector in subWidget.style) {
                setStyle(index, subIndex, styleSelector, subWidget.style[styleSelector]);
            }
            Connection.subWidget(subWidget.type, index, subIndex, true, subWidget);
        }
        return true;
    }

    function removeWidget(index: int, subIndex: int): void {
        remove(index, subIndex);
        Connection.remove(index, subIndex);
    }

    function startDrag(row: int, column: int): void {
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
        let reloadRequired = false;
        if (dragTarget.rowSpan !== block.rowSpan || dragTarget.columnSpan !== block.columnSpan)
            reloadRequired = true;
        let data = {
            row: dragTarget.row,
            column: dragTarget.column,
            rowSpan: dragTarget.rowSpan,
            columnSpan: dragTarget.columnSpan,
        }
        controlGrid.remove(dragTarget.index, 0);
        controlGrid.addWidget(block.type, toIndex, toIndex + ((dragTarget.rowSpan - 1) * columns) + (dragTarget.columnSpan - 1));
        for (let i = 0; i < block.subWidgets.length; i++) {
            const subWidget = block.subWidgets[i];
            if (subWidget.type === "Image" && (!subWidget.image || !subWidget.image.imageKey || subWidget.image.imageKey.length <= 1)) continue;
            const subIndex = controlGrid.subWidget(subWidget.type, toIndex, 0, subWidget);
            if (subIndex !== i+1) break;
        }

        updateMacros(
            toIndex, block,
            () => {
                data.style = block.style;
                reloadRequired = true;
            },
            () => {
                data.subWidgets = block.subWidgets;
                reloadRequired = true;
            }
        );

        for (const styleSelector in block.style)
            controlGrid.setStyle(toIndex, 0, styleSelector, block.style[styleSelector]);
        for (let i = 0; i < block.subWidgets.length; i++) {
            const subWidget = block.subWidgets[i];
            if (subWidget.type === "Image" && (!subWidget.image || !subWidget.image.imageKey || subWidget.image.imageKey.length <= 1)) continue;
            const subIndex = i + 1;
            for (const styleSelector in subWidget.style)
                setStyle(toIndex, subIndex, styleSelector, subWidget.style[styleSelector]);
        }

        if (reloadRequired) {
            Connection.remove(dragTarget.index, 0);
            Connection.addWidget(block.type, toIndex, toIndex + ((dragTarget.rowSpan - 1) * columns) + (dragTarget.columnSpan - 1), block.style);
            for (let i = 0; i < block.subWidgets.length; i++) {
                const subWidget = block.subWidgets[i];
                if (subWidget.type === "Image" && (!subWidget.image || !subWidget.image.imageKey || subWidget.image.imageKey.length <= 1)) continue;
                Connection.subWidget(subWidget.type, toIndex, i+1, true, subWidget);
            }
        } else {
            if (toIndex !== dragTarget.index)
                Connection.move(dragTarget.index, toIndex);
            if (dragTarget.rowSpan !== block.rowSpan || dragTarget.columnSpan !== block.columnSpan)
                Connection.changeSize(toIndex, toIndex + ((dragTarget.rowSpan - 1) * columns) + (dragTarget.columnSpan - 1));
        }
        settings.editBlock(layoutName, block.row, block.column, data);
    }

}