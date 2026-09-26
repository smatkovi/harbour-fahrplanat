import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
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
    property int fontSize: AppTheme.fontSizeMedium
    property string baseColor: ""

    property bool deviates: hasRealtime && delay !== 0 && realtime !== "" && realtime !== scheduled
    property color onTimeColor: "#2e9e46"

    spacing: AppTheme.paddingSmall

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
            return root.delay > 0 ? AppTheme.errorColor : root.onTimeColor
        }
    }
    Label {
        visible: root.deviates && !root.cancelled
        anchors.verticalCenter: parent.verticalCenter
        text: root.scheduled
        font.pixelSize: AppTheme.fontSizeExtraSmall
        font.strikeout: true
        color: palette.secondaryColor
    }
}
