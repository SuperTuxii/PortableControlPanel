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