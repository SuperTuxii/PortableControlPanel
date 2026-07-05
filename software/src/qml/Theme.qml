pragma Singleton

import QtQuick

Item {
    property color mainBackground: "#191919"
    property color secondaryBackground: "#262626"
    property color border: "#131313"
    property color secondaryBorder: "#191919"
    property real borderWidth: 2
    property real borderRadius: 10
    property color displayBorder: "#0E0E0E"
    property real displayBorderRadius: 30

    property real sidebarRadius: 25
    property real sidebarWidth: 75
    property real sidebarButtonRadius: 15
    property real sidebarButtonBorderWidth: 1.5
    property real sidebarIconSize: 35
    property real sidebarSeperatorWidth: 5
    property real sidebarSeperatorMargin: 5
    property real sidebarSeperatorBorderWidth: 1

    property color sliderActiveBackground: "#313131"

    property real buttonBackgroundDarker: 1.16
    property real buttonBorderDarker: 1.16
    property real buttonBorderWidth: 1
    property real buttonRadius: 5
    property color buttonRed: "#ff4040"
    property color buttonRedActive: "#b31212"
    property color buttonGreen: "#59ff59"
    property color buttonGreenActive: "#1fcc1f"

    property color labelWhite: "#EEEEEE"
    property color labelRed: "#c34141"

    property color textFieldBackground: "#1E1E1E"
    property color textFieldBorder: "#161616"
    property real textFieldBorderWidth: 1
    property real textFieldRadius: 2.5

    property color placeholderBlockColor: "#191919"
    property real placeholderBlockOpacity: 0.4
    property real placeholderBlockRadius: 10

    property alias montserratFontName: montserratFont.name
    property int montserratFontWeight: 400

    property alias iconFontName: materialFont.name
    property int iconFontWeight: 400
    property real iconFontSize: 24
    property QtObject icons: QtObject {
        readonly property string add: "\ue145"
        readonly property string trash: "\ue872"
        readonly property string remove: "\ue15b"
        readonly property string save: "\ue161"
        readonly property string connection: "\uf0c1"
        readonly property string noConnection: "\uf7b4"
        readonly property string brightness1: "\ue3aa"
        readonly property string brightness2: "\ue3ab"
        readonly property string brightness3: "\ue3ac"
    }
    property QtObject toast: QtObject {
        property color accentInfo: "#3498DB"
        property color accentSuccess: "#07BC0C"
        property color accentWarning: "#F1C40F"
        property color accentError: "#E74C3C"
        property color lightBackground: "#FFFFFF"
        property color darkBackground: "#121212"
        property color shadowColor: "#1A000000"
        property int lightFontWeight: Font.Normal
        property int darkFontWeight: Font.Medium
        property int coloredFontWeight: Font.Medium
        property color lightFontColor: "#000000"
        property color darkFontColor: "#FFFFFF"
        property color coloredFontColor: "#FFFFFF"
        property real lightProgressbarAccentDarker: 1.0
        property real darkProgressbarAccentDarker: 1.5
        property color coloredProgressbar: "#9Cf5f5f5"
    }

    FontLoader {
        id: montserratFont
        source: "qrc:///fonts/Montserrat.ttf"
    }
    FontLoader {
        id: materialFont
        source: "qrc:///fonts/MaterialSymbolsRounded.ttf"
    }
}