import QtQuick
import QtQuick.Effects
import QtQuick.Controls

Item {
    id: root
    property real blockSize
    property var style: {}

    RectangularShadow { // Shadow
        anchors.fill: button
        radius: button.background.radius
        offset.x: (root.getStyleProp(Connection.ShadowOffsetX, Connection.PartMain) ?? [0])[0]
        offset.y: {
            let offsets = (root.getStyleProp(Connection.ShadowOffsetX, Connection.PartMain) ?? [0]);
            return offsets.length === 1 ? offsets[0] : offsets[1];
        }
        spread: root.getStyleProp(Connection.ShadowSpread, Connection.PartMain) ??  0
        blur: root.getStyleProp(Connection.ShadowWidth, Connection.PartMain) ??  0
        color: root.getStyleProp(Connection.ShadowColor, Connection.PartMain) ?? "#00000000"
    }

    Button {
        id: button

        anchors.fill: parent
        // Margins
        anchors.topMargin: (root.getStyleProp(Connection.MarginAll, Connection.PartMain) ?? [0])[0];
        anchors.bottomMargin: {
            let margins = (root.getStyleProp(Connection.MarginAll, Connection.PartMain) ?? [0]);
            return margins.length <= 2 ? margins[0] : margins[2];
        }
        anchors.rightMargin: {
            let margins = (root.getStyleProp(Connection.MarginAll, Connection.PartMain) ?? [0]);
            return margins.length === 1 ? margins[0] : margins.length === 2 ? margins[1] : margins[1];
        }
        anchors.leftMargin: {
            let margins = (root.getStyleProp(Connection.MarginAll, Connection.PartMain) ?? [0]);
            return margins.length === 1 ? margins[0] : margins.length === 2 ? margins[1] : margins[3];
        }

        // Opacity
        opacity: (root.getStyleProp(Connection.Opacity, Connection.PartMain) ?? 255) / 255

        // Padding
        topPadding: (root.getStyleProp(Connection.PadAll, Connection.PartMain) ?? [0])[0];
        bottomPadding: {
            let pads = (root.getStyleProp(Connection.PadAll, Connection.PartMain) ?? [0]);
            return pads.length <= 2 ? pads[0] : pads[2];
        }
        rightPadding: {
            let pads = (root.getStyleProp(Connection.PadAll, Connection.PartMain) ?? [0]);
            return pads.length === 1 ? pads[0] : pads.length === 2 ? pads[1] : pads[1];
        }
        leftPadding: {
            let pads = (root.getStyleProp(Connection.PadAll, Connection.PartMain) ?? [0]);
            return pads.length === 1 ? pads[0] : pads.length === 2 ? pads[1] : pads[3];
        }

        // Text
        font.family: Theme.montserratFontName
        font.weight: Theme.montserratFontWeight
        font.pixelSize: root.getStyleProp(Connection.FontMontserrat14, Connection.PartMain) ? 14 : root.getStyleProp(Connection.FontMontserrat48, Connection.PartMain) ? 48 : root.getStyleProp(Connection.FontSizeScaled, Connection.PartMain) ?? 14
        font.letterSpacing: root.getStyleProp(Connection.TextLetterSpace, Connection.PartMain) ?? 0

        background: Rectangle { // Background & Border
            color: root.getStyleProp(Connection.BackgroundColor, Connection.PartMain) ?? "#2095f6"
            radius: root.getStyleProp(Connection.Radius, Connection.PartMain) ?? 10
            border.color: root.getStyleProp(Connection.BorderColor, Connection.PartMain) ?? "#FFFFFF"
            border.width: root.getStyleProp(Connection.BorderWidth, Connection.PartMain) ?? 0
            layer.enabled: true
            layer.effect: MultiEffect { // Drop Shadow
                shadowEnabled: root.getStyleProp(Connection.DropShadowRadius, Connection.PartMain) !== undefined || root.getStyleProp(Connection.DropShadowColor, Connection.PartMain) !== undefined || root.getStyleProp(Connection.DropShadowOffsetX, Connection.PartMain) !== undefined
                blurEnabled: root.getStyleProp(Connection.BlurRadius, Connection.PartMain) !== undefined
                blurMax: Math.max(root.getStyleProp(Connection.DropShadowRadius, Connection.PartMain) ?? 0, root.getStyleProp(Connection.BlurRadius, Connection.PartMain) ?? 0, 1)
                blur: (root.getStyleProp(Connection.BlurRadius, Connection.PartMain) ?? 0.0) / blurMax
                shadowColor: root.getStyleProp(Connection.DropShadowColor, Connection.PartMain) ?? "#00000000"
                shadowBlur: (root.getStyleProp(Connection.DropShadowRadius, Connection.PartMain) ?? 0) / blurMax
                shadowHorizontalOffset: (root.getStyleProp(Connection.DropShadowOffsetX, Connection.PartMain) ?? [0])[0]
                shadowVerticalOffset: {
                    let offsets = (root.getStyleProp(Connection.DropShadowOffsetX, Connection.PartMain) ?? [0]);
                    return offsets.length === 1 ? offsets[0] : offsets[1];
                }
            }
        }

        Rectangle { // Outline
            anchors.fill: parent
            anchors.margins: -border.width - (root.getStyleProp(Connection.OutlinePad, Connection.PartMain) ?? 0)
            color: "#00000000"
            radius: root.getStyleProp(Connection.Radius, Connection.PartMain) ?? 10 - anchors.margins
            border.color: root.getStyleProp(Connection.OutlineColor, Connection.PartMain) ?? "#FFFFFF"
            border.width: root.getStyleProp(Connection.OutlineWidth, Connection.PartMain) ?? 0
        }
    }

    function getStyleState() {
        if (button.checked)
            return Connection.StateChecked;
        if (button.pressed)
            return Connection.StatePressed;
        if (!button.enabled)
            return Connection.StateDisabled;
        return Connection.StateDefault;
    }
    function getStyleProp(prop, part) {
        if (!style) return undefined;
        let state = getStyleState();
        if ((part | state) in style && prop in style[(part | state)])
            return style[(part | state)][prop];
        if ((part | Connection.StateAny) in style && prop in style[(part | Connection.StateAny)])
            return style[(part | Connection.StateAny)][prop];
        if (state !== Connection.StateDefault && (part | Connection.StateDefault) in style && prop in style[(part | Connection.StateDefault)])
            return style[(part | Connection.StateDefault)][prop];
        if ((Connection.PartAny | state) in style && prop in style[(Connection.PartAny | state)])
            return style[(Connection.PartAny | state)][prop];
        if ((Connection.PartAny | Connection.StateAny) in style && prop in style[(Connection.PartAny | Connection.StateAny)])
            return style[(Connection.PartAny | Connection.StateAny)][prop];
        if (state !== Connection.StateDefault && (Connection.PartAny | Connection.StateDefault) in style && prop in style[(Connection.PartAny | Connection.StateDefault)])
            return style[(Connection.PartAny | Connection.StateDefault)][prop];
    }
}