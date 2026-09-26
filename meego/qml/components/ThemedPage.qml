import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"

// Die gemeinsame Seite aller Ansichten.
//
// Auf Sailfish setzt sie Silicas palette und laesst die Komponenten die Farben
// daraus ziehen. Qt 4.7 hat keine palette an der Seite -- also ist sie hier
// eine gewoehnliche Eigenschaft dieses Objekts. Die Seiten schreiben
// palette.secondaryColor wie vorher, und der Name loest sich im Bereich der
// Seite auf.
Page {
    id: themedPage

    property int colorTheme: settings.colorTheme
    property bool customTheme: colorTheme !== 0
    property bool lightTheme: colorTheme === 1
    property color accentColor: lightTheme ? "#c8102e" : "#ff3d55"
    // Der dunkle Grund ist 000000. Auf dem N9/N950 ist das kein Geschmack,
    // sondern ein ausgeschaltetes Pixel.
    property color pageBackground: lightTheme ? "#f7f7f7" : "#000000"

    property QtObject palette: QtObject {
        property color primaryColor: themedPage.lightTheme ? "#1f1f1f" : "#f2f2f2"
        property color secondaryColor: themedPage.lightTheme ? "#6a6a6a" : "#a9a9a9"
        property color highlightColor: themedPage.accentColor
        property color secondaryHighlightColor: themedPage.lightTheme ? "#d9566a" : "#ff8a99"
        property color highlightBackgroundColor: themedPage.accentColor
        property color highlightDimmerColor: themedPage.lightTheme ? "#7a0a1d" : "#5a0a16"
        property int colorScheme: themedPage.lightTheme ? 1 : 0
    }

    orientationLock: PageOrientation.Automatic

    Rectangle {
        anchors.fill: parent
        color: themedPage.pageBackground
        z: -1
    }
}
