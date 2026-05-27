import QtQuick
import QtQuick.Layouts

Rectangle {
    id: background
    property bool fillPlaceholders: true
    property alias rows: layout.rows
    property alias columns: layout.columns
    property alias rowSpacing: layout.rowSpacing
    property alias columnSpacing: layout.columnSpacing
    property real outerPad: 5
    readonly property real layoutHeight: rows * blockSize + (rows - 1) * layout.rowSpacing
    readonly property real layoutWidth: columns * blockSize + (columns - 1) * layout.columnSpacing
    readonly property alias blockSize: layout.blockSize

    color: Theme.displayBackground
    anchors.centerIn: parent


    GridLayout {
        id: layout

        property real blockSize: Math.min(
            Math.floor((background.width - (2 * background.outerPad) - (layout.columnSpacing * (layout.columns - 1))) / layout.columns),
            Math.floor((background.height - (2 * background.outerPad) - (layout.rowSpacing * (layout.rows - 1))) / layout.rows)
        )

        rows: 3
        columns: 5
        rowSpacing: 5
        columnSpacing: 5
        anchors.centerIn: parent
    }

    Component.onCompleted: {
        fillLayout("blocks/PlaceholderBlock.qml", { opacity: fillPlaceholders ? Theme.placeholderBlockOpacity : 0 });
    }

    function fillLayout(componentPath, data): void {
        clearLayout();
        const component = Qt.createComponent(componentPath);
        if (component.status === Component.Error) {
            console.error(component.errorString());
        } else if (component.status === Component.Ready) {
            data.blockSize = Qt.binding(function() { return layout.blockSize; });
            data.implicitHeight = layout.blockSize;
            data.implicitWidth = layout.blockSize;
            for (let i = 0; i < layout.rows * layout.columns; i++) {
                data["Layout.column"] = i % layout.columns;
                data["Layout.row"] = Math.floor(i / layout.columns);
                component.createObject(layout, data);
            }
        } else {
            console.error("component not ready yet");
        }
    }

    function clearLayout(): void {
        while (layout.children.length !== 0) {
            layout.children[0].destroy();
            layout.children[0].parent = null;
        }
    }
}