import QtQuick 2.6
import Sailfish.Silica 1.0

// Shows the effective time. With real-time data the time is green when on
// schedule and red when late; the planned time is shown struck through when
// the real-time value deviates. baseColor may be left empty to use the
// palette's primary colour.
Row {
    id: root

    property string scheduled
    property string realtime
    property bool hasRealtime: false
    property int delay: 0
    property bool cancelled: false
    property int fontSize: Theme.fontSizeMedium
    property string baseColor: ""

    readonly property bool deviates: hasRealtime && delay !== 0 && realtime !== "" && realtime !== scheduled
    readonly property color onTimeColor: "#2e9e46"

    spacing: Theme.paddingSmall

    Label {
        text: root.deviates ? root.realtime : root.scheduled
        font.pixelSize: root.fontSize
        font.strikeout: root.cancelled
        color: {
            if (root.cancelled) {
                return palette.secondaryColor
            }
            if (!root.hasRealtime) {
                return root.baseColor !== "" ? root.baseColor : palette.primaryColor
            }
            return root.delay > 0 ? Theme.errorColor : root.onTimeColor
        }
    }
    Label {
        visible: root.deviates && !root.cancelled
        anchors.verticalCenter: parent.verticalCenter
        text: root.scheduled
        font.pixelSize: Theme.fontSizeExtraSmall
        font.strikeout: true
        color: palette.secondaryColor
    }
}
