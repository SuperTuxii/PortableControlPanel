import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: layout
    readonly property var structure: {
        let structure = {
            "Transform": [
                { text: "Size", value: Connection.TransformWidth },
                { text: "Position", value: Connection.TranslateX },
                { text: "Radial", value: Connection.TranslateRadial },
                { text: "Scale", value: Connection.TranslateScale },
                { text: "Align Pivot", prependFromParent: false, value: Connection.AlignTransformPivot },
                { text: "Rotation", value: Connection.TransformRotation },
                { text: "Pivot", value: Connection.TransformPivotX },
                { text: "Skew", value: Connection.TransformSkewX },
            ],
            "Padding": Connection.PadAll,
            "Margin": Connection.MarginAll,
            "Background": [
                { text: "Color", value: Connection.BackgroundColor },
                { text: "Gradient", fullText: "Bg Gradient", value: [
                        { text: "Color", value: Connection.BackgroundGradColor },
                        { text: "Main Opacity", fullText: "Bg Gradient Main Opa", value: Connection.BackgroundMainOpacity },
                        { text: "Main Stop", value: Connection.BackgroundMainStop },
                        { text: "Stop", value: Connection.BackgroundGradStop },
                        { text: "Direction", value: Connection.BackgroundGradDirNone },
                        { text: "Extend", value: Connection.BackgroundGradExtendPad },
                        { text: "Linear", prependToChildText: false, value: [
                                { text: "Start", value: Connection.BackgroundGradLinearStartX },
                                { text: "End", value: Connection.BackgroundGradLinearEndX },
                            ]
                        },
                        { text: "Radial", fullText: "Bg Grad", value: [
                                { text: "Focal", value: Connection.BackgroundGradRadialFocalX },
                                { text: "End", value: Connection.BackgroundGradRadialEndX },
                                { text: "Focal Radius", value: Connection.BackgroundGradRadialFocalRadius },
                                { text: "End Radius", value: Connection.BackgroundGradRadialEndRadius },
                            ]
                        },
                        { text: "Conical", fullText: "Bg Grad", value: [
                                { text: "Center", value: Connection.BackgroundGradConicalCenterX },
                                { text: "Start Angle", value: Connection.BackgroundGradConicalStartAngle },
                                { text: "End Angle", value: Connection.BackgroundGradConicalEndAngle },
                            ]
                        },
                    ]
                },
                { text: "Image", fullText: "Bg Image", value: [
                        { text: "Index", value: Connection.BackgroundImageIndex },
                        { text: "Opacity", value: Connection.BackgroundImageOpacity },
                        { text: "Recolor", value: Connection.BackgroundImageRecolor },
                        { text: "Tiled", value: Connection.BackgroundImageTiledOff },
                    ]
                },
            ],
            "Border": [
                { text: "Color", value: Connection.BorderColor },
                { text: "Width", value: Connection.BorderWidth },
                { text: "Side", value: Connection.BorderSideNone },
            ],
            "Outline": [
                { text: "Color", value: Connection.OutlineColor },
                { text: "Width", value: Connection.OutlineWidth },
                { text: "Pad", value: Connection.OutlinePad },
            ],
            "Shadow": [
                { text: "Color", value: Connection.ShadowColor },
                { text: "Width", value: Connection.ShadowWidth },
                { text: "Offset", value: Connection.ShadowOffsetX },
                { text: "Spread", value: Connection.ShadowSpread },
            ],
            "Image": [
                { text: "Opacity", value: Connection.ImageOpacity },
                { text: "Recolor", value: Connection.ImageRecolor },
            ],
            "Text": [
                { text: "Color", value: Connection.TextColor },
                { text: "Font", prependFromParent: false, value: [
                        { text: "Montserrat Scaled", fullText: "Font Montserrat", value: Connection.FontSizeScaled },
                        { text: "Montserrat14", value: Connection.FontMontserrat14 },
                        { text: "Montserrat48", value: Connection.FontMontserrat48 },
                    ]
                },
                { text: "Letter Space", value: Connection.TextLetterSpace },
                { text: "Line Space", value: Connection.TextLineSpace },
                { text: "Decor", value: Connection.TextDecorNone },
                { text: "Align", value: Connection.TextAlignAuto },
                { text: "Outline Color", value: Connection.TextOutlineStrokeColor },
                { text: "Outline Width", value: Connection.TextOutlineStrokeWidth },
            ],
            "Blur": [
                { text: "Radius", value: Connection.BlurRadius },
                { text: "Backdrop", value: Connection.BlurBackdropOff },
                { text: "Quality", value: Connection.BlurQualityAuto },
            ],
            "Drop Shadow": [
                { text: "Radius", value: Connection.DropShadowRadius },
                { text: "Offset", value: Connection.DropShadowOffsetX },
                { text: "Color", value: Connection.DropShadowColor },
                { text: "Quality", value: Connection.DropShadowQualityAuto },
            ],
            "Miscellaneous": [
                { text: "Align", value: Connection.AlignTopLeft },
                { text: "Radius", value: Connection.Radius },
                { text: "Radius Offset", value: Connection.RadiusOffset },
                { text: "Clip Corner", value: Connection.ClipCornerOff },
                { text: "Opacity", value: Connection.Opacity },
                { text: "Opacity Layered", value: Connection.OpacityLayered },
                { text: "Color Filter", value: Connection.ColorFilterUnset },
                { text: "Color Filter Opacity", value: Connection.ColorFilterOpacity },
                { text: "Recolor", value: Connection.Recolor },
                { text: "Blend Mode", value: Connection.BlendModeNormal },
                { text: "Rotary Sensitivity", value: Connection.RotarySensitivity },
            ],
            "Size and Position": [],
        };
        for (let i = Connection.Width; i <= Connection.Y; i++) { // Size and Position
            structure["Size and Position"].push({ text: Connection.styleKeyString(i).replace(/(.)([A-Z])/g, "$1 $2"), value: i });
        }
        return structure;
    }

    ComboBox {
        id: styleKeyComboBox
        Layout.preferredWidth: (menuValueScroll.availableWidth - parent.spacing) - 30
        textRole: "text"
        valueRole: "value"
        model: []
        popup: Menu {
            y: styleKeyComboBox.height

            Component {
                id: menuItemComponent

                Action {
                    required property var parentMenu
                    required property int value
                    property bool prependFromParent: true
                    property string fullText: ("fullText" in parentMenu && parentMenu.fullText.length > 0 && prependFromParent ? parentMenu.fullText + " " : "") + text
                    onTriggered: styleKeyComboBox.currentValue = value
                }
            }

            Component {
                id: subMenuComponent

                Menu {
                    id: menu
                    required property var parentMenu
                    required property string text
                    required property var value
                    property bool prependToChildText: true
                    property bool prependFromParent: true
                    property string fullText: ("fullText" in parentMenu && parentMenu.fullText.length > 0 && prependFromParent ? parentMenu.fullText + (prependToChildText ? " " : "") : "") + (prependToChildText ? text : "")
                    title: text
                    function populateMenu() {
                        for (let element of value) {
                            element.parentMenu = menu;
                            if (Array.isArray(element.value) || typeof element.value === "object") {
                                menu.addMenu(subMenuComponent.createObject(menu.contentItem, element));
                            } else {
                                let action = menuItemComponent.createObject(menu.contentItem, element);
                                menu.addAction(action);
                                styleKeyComboBox.model.push({ text: action.fullText, value: element.value });
                            }
                        }
                        styleKeyComboBox.currentIndex = -1;
                    }
                    Component.onCompleted: {
                        Qt.callLater(menu.populateMenu);
                    }
                }
            }

            Component.onCompleted: {
                for (const elementKey in layout.structure) {
                    let element = { text: elementKey, value: layout.structure[elementKey], parentMenu: styleKeyComboBox };
                    if (Array.isArray(element.value)) {
                        if (element.text === "Miscellaneous" || element.text === "Size and Position")
                            element.prependToChildText = false;
                        addMenu(subMenuComponent.createObject(contentItem, element));
                    } else {
                        let action = menuItemComponent.createObject(contentItem, element);
                        addAction(action);
                        styleKeyComboBox.model.push({ text: action.fullText, value: element.value });
                    }
                }
                styleKeyComboBox.currentIndex = -1;
            }
        }
        background: Rectangle {
            color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
            radius: Theme.buttonRadius
            border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
            border.width: Theme.buttonBorderWidth
        }
    }
    Button {
        Layout.preferredWidth: 30
        Layout.preferredHeight: 30
        Layout.alignment: Qt.AlignCenter
        text: Theme.icons.add
        font.family: Theme.iconFontName
        font.weight: Theme.iconFontWeight
        font.pixelSize: Theme.iconFontSize
        enabled: styleKeyComboBox.currentIndex !== -1
        background: Rectangle {
            color: Qt.darker(Theme.mainBackground, parent.down ? Theme.buttonBackgroundDarker : parent.hovered ? 1 / Theme.buttonBackgroundDarker : 1)
            radius: Theme.buttonRadius
            border.color: Qt.darker(Theme.border, parent.down ? Theme.buttonBorderDarker : parent.hovered ? 1 / Theme.buttonBorderDarker : 1)
            border.width: Theme.buttonBorderWidth
        }
        onClicked: {
            popup.addStyleKeyValue(styleKeyComboBox.currentValue, styleKeyComboBox.currentText);
        }
    }
}