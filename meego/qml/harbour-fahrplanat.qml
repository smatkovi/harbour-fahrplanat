import QtQuick 1.1
import com.nokia.meego 1.0
import "pages"

// Das Wurzelobjekt. Auf Sailfish ist es ein ApplicationWindow mit Cover;
// Harmattan kennt kein Cover und stapelt Seiten in einem PageStackWindow.
//
// Es heisst appWindow, weil die Ersatzkomponenten es brauchen: ein Menue
// haengt seine Flaeche ans Elternteil, und eine ContextMenu, die einer
// Eigenschaft zugewiesen wird, hat gar keins -- die Menues haengen deshalb
// hier.
PageStackWindow {
    id: appWindow

    showStatusBar: true
    // Mit Leiste, sonst kommt man aus einer Unterseite nicht mehr zurueck:
    // Harmattan hat keine Rueckwaerts-Geste, der Weg zurueck ist der Pfeil
    // links unten. Den Eintrag dafuer bringt jede Seite mit (ThemedPage).
    showToolBar: true

    // Der Grund ist schwarz, nicht "fast schwarz": auf dem OLED bleiben die
    // Pixel damit aus.
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        z: -1
    }

    Component.onCompleted: theme.inverted = true

    initialPage: SearchPage { }
}
