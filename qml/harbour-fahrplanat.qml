import QtQuick 2.6
import Sailfish.Silica 1.0
import "pages"

ApplicationWindow {
    id: app

    initialPage: Component { SearchPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
