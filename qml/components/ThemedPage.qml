import QtQuick 2.6
import Sailfish.Silica 1.0

// Page with the app colour scheme: red accent on a light or dark background
// (settings.colorTheme 1 / 2) or the plain ambience palette (0). Silica
// components pick the colours up through the palette; own items use
// palette.* as well.
Page {
    id: themedPage

    readonly property int colorTheme: settings.colorTheme
    readonly property bool customTheme: colorTheme !== 0
    readonly property bool lightTheme: colorTheme === 1
    readonly property color accentColor: lightTheme ? "#c8102e" : "#ff3d55"
    readonly property color pageBackground: lightTheme ? "#f7f7f7" : "#141416"

    palette.colorScheme: customTheme ? (lightTheme ? Theme.DarkOnLight : Theme.LightOnDark) : Theme.colorScheme
    palette.primaryColor: customTheme ? (lightTheme ? "#1f1f1f" : "#f2f2f2") : Theme.primaryColor
    palette.secondaryColor: customTheme ? (lightTheme ? "#6a6a6a" : "#a9a9a9") : Theme.secondaryColor
    palette.highlightColor: customTheme ? accentColor : Theme.highlightColor
    palette.secondaryHighlightColor: customTheme ? (lightTheme ? "#d9566a" : "#ff8a99") : Theme.secondaryHighlightColor
    palette.highlightBackgroundColor: customTheme ? accentColor : Theme.highlightBackgroundColor
    palette.highlightDimmerColor: customTheme ? (lightTheme ? "#7a0a1d" : "#5a0a16") : Theme.highlightDimmerColor

    Rectangle {
        anchors.fill: parent
        visible: themedPage.customTheme
        color: themedPage.pageBackground
        z: -1
    }
}
