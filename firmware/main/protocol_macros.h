#ifndef CONTROLPANELSOFTWARE_PROTOCOL_MACROS_H
#define CONTROLPANELSOFTWARE_PROTOCOL_MACROS_H

#define PROTOCOL_COMMANDS_ENUM enum Commands {\
    /* No Operands & No Data */\
    RestartCMD = 0x00,\
    DeepSleepCMD,\
    TestFillCMD,\
    ClearCMD,\
    /* Operands & No Data*/\
    SetLayoutCMD = 0x40,\
    MoveWidgetCMD,\
    ChangeWidgetSizeCMD,\
    RemoveWidgetCMD,\
    RemoveImageCMD,\
    /* No Operands & Data */\
    SetOuterPadCMD = 0x80,\
    SetRowPadCMD,\
    SetColumnPadCMD,\
    /* Operands & Data */\
    SetStyleDataCMD = 0xC0,\
    AddImageCMD,\
    ModifyImageCMD,\
    CreateButtonCMD,\
    SubTextCMD,\
    SubImageCMD,\
};

#define PROTOCOL_STYLE_KEYS_ENUM enum StyleKeys {\
    /* Special Keys */\
    SetStyleSelector,\
    ConsiderScale,\
    /* Number Keys */\
    PadAll,\
    PadTop,\
    PadBottom,\
    PadLeft,\
    PadRight,\
    MarginAll,\
    MarginTop,\
    MarginBottom,\
    MarginLeft,\
    MarginRight,\
    BorderWidth,\
    OutlineWidth,\
    OutlinePad,\
    ShadowWidth,\
    ShadowOffsetX,\
    ShadowOffsetY,\
    ShadowSpread,\
    TextLetterSpace,\
    TextLineSpace,\
    TextOutlineStrokeWidth,\
    BlurRadius,\
    DropShadowRadius,\
    DropShadowOffsetX,\
    DropShadowOffsetY,\
    Width,\
    MinWidth,\
    MaxWidth,\
    Height,\
    MinHeight,\
    MaxHeight,\
    Length,\
    X,\
    Y,\
    TransformWidth,\
    TransformHeight,\
    TranslateX,\
    TranslateY,\
    TranslateRadial,\
    TranslateScale,\
    TranslateScaleX,\
    TranslateScaleY,\
    TransformRotation,\
    TransformPivotX,\
    TransformPivotY,\
    TransformSkewX,\
    TransformSkewY,\
    Radius,\
    RadiusOffset,\
    RotarySensitivity,\
    BackgroundGradParams1,\
    BackgroundGradParams2,\
    BackgroundGradLinearStartX = BackgroundGradParams1,\
    BackgroundGradLinearStartY = BackgroundGradParams2,\
    BackgroundGradRadialFocalX = BackgroundGradParams1,\
    BackgroundGradRadialFocalY = BackgroundGradParams2,\
    BackgroundGradConicalCenterX = BackgroundGradParams1,\
    BackgroundGradConicalCenterY = BackgroundGradParams2,\
    BackgroundGradLinearEndX,\
    BackgroundGradLinearEndY,\
    BackgroundGradRadialEndX,\
    BackgroundGradRadialEndY,\
    BackgroundGradRadialFocalRadius,\
    BackgroundGradRadialEndRadius,\
    /* Color + Opacity Keys */\
    BackgroundColor,\
    BackgroundGradColor,\
    BackgroundImageRecolor,\
    BorderColor,\
    OutlineColor,\
    ShadowColor,\
    TextColor,\
    TextOutlineStrokeColor,\
    DropShadowColor,\
    Recolor,\
    ImageRecolor,\
    /* Number (16) Keys */\
    BackgroundGradConicalStartAngle,\
    BackgroundGradConicalEndAngle,\
    /* Byte Keys */\
    BackgroundImageIndex,\
    BackgroundMainOpacity,\
    BackgroundMainStop,\
    BackgroundGradStop,\
    BackgroundImageOpacity,\
    FontSizeScaled,\
    Opacity,\
    OpacityLayered,\
    ColorFilterOpacity,\
    ImageOpacity,\
    /* None Type Keys */\
    BackgroundGradDirNone,\
    BackgroundGradDirVer,\
    BackgroundGradDirHor,\
    BackgroundGradDirLinear,\
    BackgroundGradDirRadial,\
    BackgroundGradDirConical,\
    BackgroundGradExtendPad,\
    BackgroundGradExtendRepeat,\
    BackgroundGradExtendReflect,\
    BorderSideNone,\
    BorderSideBottom,\
    BorderSideTop,\
    BorderSideLeft,\
    BorderSideRight,\
    BorderSideFull,\
    TextDecorNone,\
    TextDecorUnderline,\
    TextDecorStrikethrough,\
    TextAlignAuto,\
    TextAlignLeft,\
    TextAlignCenter,\
    TextAlignRight,\
    BlurBackdropOff,\
    BlurBackdropOn,\
    BlurQualityAuto,\
    BlurQualitySpeed,\
    BlurQualityPrecision,\
    DropShadowQualityAuto,\
    DropShadowQualitySpeed,\
    DropShadowQualityPrecision,\
    AlignTopLeft,\
    AlignTopMid,\
    AlignTopRight,\
    AlignBottomLeft,\
    AlignBottomMid,\
    AlignBottomRight,\
    AlignLeftMid,\
    AlignCenter,\
    AlignRightMid,\
    AlignTransformPivot,\
    AlignTransformPivotAll,\
    AlignTransformPivotEvent,\
    AlignTransformPivotAllEvent,\
    FontMontserrat14,\
    FontMontserrat48,\
    BackgroundImageTiledOff,\
    BackgroundImageTiledOn,\
    ClipCornerOff,\
    ClipCornerOn,\
    ColorFilterUnset,\
    ColorFilterShade,\
    BlendModeNormal,\
    BlendModeAdditive,\
    BlendModeSubtractive,\
    BlendModeMultiply,\
    BlendModeDifference,\
    BaseDirLTR,\
    BaseDirRTL,\
    BaseDirAuto,\
    /* Control Values */\
    NumberStyleKeyMin = PadAll,\
    NumberStyleKeyMax = BackgroundGradRadialEndRadius,\
    ColorOpacityStyleKeyMin = NumberStyleKeyMax + 1,\
    ColorOpacityStyleKeyMax = ImageRecolor,\
    Number16StyleKeyMin = ColorOpacityStyleKeyMax + 1,\
    Number16StyleKeyMax = BackgroundGradConicalEndAngle,\
    ByteStyleKeyMin = Number16StyleKeyMax + 1,\
    ByteStyleKeyMax = ImageOpacity,\
    NonTypeStyleKeyMin = ByteStyleKeyMax + 1,\
    NonTypeStyleKeyMax = BaseDirAuto\
};

#define PROTOCOL_STYLE_STATES_ENUM enum StyleStates {\
    StateDefault        = LV_STATE_DEFAULT,\
    StateAlt            = LV_STATE_ALT,\
    StateChecked        = LV_STATE_CHECKED,\
    StateFocused        = LV_STATE_FOCUSED,\
    StateFocusKey       = LV_STATE_FOCUS_KEY,\
    StateEdited         = LV_STATE_EDITED,\
    StateHovered        = LV_STATE_HOVERED,\
    StatePressed        = LV_STATE_PRESSED,\
    StateScrolled       = LV_STATE_SCROLLED,\
    StateDisabled       = LV_STATE_DISABLED,\
    StateUser1          = LV_STATE_USER_1,\
    StateUser2          = LV_STATE_USER_2,\
    StateUser3          = LV_STATE_USER_3,\
    StateUser4          = LV_STATE_USER_4,\
    StateAny            = LV_STATE_ANY,\
};

#define PROTOCOL_STYLE_PARTS_ENUM enum StyleParts {\
    PartMain            = LV_PART_MAIN,\
    PartScrollbar       = LV_PART_SCROLLBAR,\
    PartIndicator       = LV_PART_INDICATOR,\
    PartKnob            = LV_PART_KNOB,\
    PartSelected        = LV_PART_SELECTED,\
    PartItems           = LV_PART_ITEMS,\
    PartCursor          = LV_PART_CURSOR,\
    PartCustomFirst     = LV_PART_CUSTOM_FIRST,\
    PartAny             = LV_PART_ANY,\
};

#endif //CONTROLPANELSOFTWARE_PROTOCOL_MACROS_H
