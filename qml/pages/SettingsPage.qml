import QtQuick 2.6
import Sailfish.Silica 1.0
import "../components"

ThemedPage {
    id: page

    allowedOrientations: Orientation.All

    // Re-sync editable fields after a preset has been applied
    Connections {
        target: settings
        onProfileChanged: page.syncFields()
    }

    function syncFields() {
        endpointField.text = settings.endpoint
        versionField.text = settings.version
        extField.text = settings.ext
        languageField.text = settings.language
        clientTypeField.text = settings.clientType
        clientIdField.text = settings.clientId
        clientVersionField.text = settings.clientVersion
        clientNameField.text = settings.clientName
        aidField.text = settings.aid
        userAgentField.text = settings.userAgent
    }

    Component.onCompleted: syncFields()

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: "Profil auf Voreinstellung zurücksetzen"
                onClicked: settings.applyPreset(settings.preset)
            }
        }

        Column {
            id: column
            width: page.width

            PageHeader { title: "Einstellungen" }

            SectionHeader { text: "Darstellung" }

            ComboBox {
                property bool ready: false
                readonly property var themeValues: [1, 2, 0]
                label: "Farbschema"
                currentIndex: themeValues.indexOf(settings.colorTheme) >= 0 ? themeValues.indexOf(settings.colorTheme) : 0
                menu: ContextMenu {
                    MenuItem { text: "Rot auf hellem Grund" }
                    MenuItem { text: "Rot auf dunklem Grund" }
                    MenuItem { text: "Ambience (Systemfarben)" }
                }
                Component.onCompleted: ready = true
                onCurrentIndexChanged: {
                    if (ready && currentIndex >= 0) {
                        settings.colorTheme = themeValues[currentIndex]
                    }
                }
            }

            SectionHeader { text: "Fahrplanserver (HAFAS)" }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * x
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: palette.secondaryColor
                text: "Die Voreinstellung entspricht der Konfiguration der aktuellen ÖBB-Android-App. "
                      + "Falls der Server Anfragen ablehnt, ein anderes Profil wählen oder die Werte anpassen."
            }

            ComboBox {
                id: presetBox
                property bool ready: false
                label: "Profil"
                currentIndex: settings.preset
                menu: ContextMenu {
                    Repeater {
                        model: settings.presetCount()
                        MenuItem { text: settings.presetName(index) }
                    }
                }
                Component.onCompleted: ready = true
                onCurrentIndexChanged: {
                    if (ready && currentIndex >= 0 && currentIndex !== settings.preset) {
                        settings.applyPreset(currentIndex)
                    }
                }
            }

            TextField {
                id: endpointField
                width: parent.width
                label: "Endpoint"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhUrlCharactersOnly
                onTextChanged: settings.endpoint = text
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: versionField.focus = true
            }
            TextField {
                id: versionField
                width: parent.width
                label: "HCI-Version (ver)"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onTextChanged: settings.version = text
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: extField.focus = true
            }
            TextField {
                id: extField
                width: parent.width
                label: "Extension (ext)"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onTextChanged: settings.ext = text
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: languageField.focus = true
            }
            TextField {
                id: languageField
                width: parent.width
                label: "Sprache (lang)"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onTextChanged: settings.language = text
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: clientTypeField.focus = true
            }
            TextField {
                id: clientTypeField
                width: parent.width
                label: "Client-Typ (AND, IPH, WEB)"
                inputMethodHints: Qt.ImhNoPredictiveText
                onTextChanged: settings.clientType = text
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: clientIdField.focus = true
            }
            TextField {
                id: clientIdField
                width: parent.width
                label: "Client-ID"
                inputMethodHints: Qt.ImhNoPredictiveText
                onTextChanged: settings.clientId = text
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: clientVersionField.focus = true
            }
            TextField {
                id: clientVersionField
                width: parent.width
                label: "Client-Version (v)"
                inputMethodHints: Qt.ImhNoPredictiveText
                onTextChanged: settings.clientVersion = text
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: clientNameField.focus = true
            }
            TextField {
                id: clientNameField
                width: parent.width
                label: "Client-Name"
                inputMethodHints: Qt.ImhNoPredictiveText
                onTextChanged: settings.clientName = text
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: aidField.focus = true
            }
            TextField {
                id: aidField
                width: parent.width
                label: "Auth-AID"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                onTextChanged: settings.aid = text
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: userAgentField.focus = true
            }
            TextField {
                id: userAgentField
                width: parent.width
                label: "User-Agent"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onTextChanged: settings.userAgent = text
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
                EnterKey.onClicked: focus = false
            }

            TextSwitch {
                text: "Anfragen protokollieren"
                description: "Schreibt Anfragen und Antworten ins Journal (journalctl -f), nur zur Fehlersuche"
                automaticCheck: false
                checked: settings.logRequests
                onClicked: settings.logRequests = !settings.logRequests
            }

            SectionHeader { text: "Verlauf" }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Ortsverlauf löschen"
                onClicked: recentModel.clearHistory()
            }

            Item { width: 1; height: Theme.paddingLarge }
        }
        VerticalScrollDecorator { }
    }
}
