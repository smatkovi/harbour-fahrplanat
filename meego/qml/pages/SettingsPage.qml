import QtQuick 1.1
import com.nokia.meego 1.0
import "../silica"
import "../components"

ThemedPage {
    id: page
    orientationLock: PageOrientation.Automatic
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

    Flickable {
        anchors.fill: parent
        pressDelay: 150
        contentHeight: column.height + AppTheme.paddingLarge

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
                property variant themeValues: [1, 2, 0]
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
                x: AppTheme.horizontalPageMargin
                width: parent.width - 2 * x
                wrapMode: Text.Wrap
                font.pixelSize: AppTheme.fontSizeExtraSmall
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
                Keys.onReturnPressed: versionField.focus = true
            }
            TextField {
                id: versionField
                width: parent.width
                label: "HCI-Version (ver)"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onTextChanged: settings.version = text
                Keys.onReturnPressed: extField.focus = true
            }
            TextField {
                id: extField
                width: parent.width
                label: "Extension (ext)"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onTextChanged: settings.ext = text
                Keys.onReturnPressed: languageField.focus = true
            }
            TextField {
                id: languageField
                width: parent.width
                label: "Sprache (lang)"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onTextChanged: settings.language = text
                Keys.onReturnPressed: clientTypeField.focus = true
            }
            TextField {
                id: clientTypeField
                width: parent.width
                label: "Client-Typ (AND, IPH, WEB)"
                inputMethodHints: Qt.ImhNoPredictiveText
                onTextChanged: settings.clientType = text
                Keys.onReturnPressed: clientIdField.focus = true
            }
            TextField {
                id: clientIdField
                width: parent.width
                label: "Client-ID"
                inputMethodHints: Qt.ImhNoPredictiveText
                onTextChanged: settings.clientId = text
                Keys.onReturnPressed: clientVersionField.focus = true
            }
            TextField {
                id: clientVersionField
                width: parent.width
                label: "Client-Version (v)"
                inputMethodHints: Qt.ImhNoPredictiveText
                onTextChanged: settings.clientVersion = text
                Keys.onReturnPressed: clientNameField.focus = true
            }
            TextField {
                id: clientNameField
                width: parent.width
                label: "Client-Name"
                inputMethodHints: Qt.ImhNoPredictiveText
                onTextChanged: settings.clientName = text
                Keys.onReturnPressed: aidField.focus = true
            }
            TextField {
                id: aidField
                width: parent.width
                label: "Auth-AID"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                onTextChanged: settings.aid = text
                Keys.onReturnPressed: userAgentField.focus = true
            }
            TextField {
                id: userAgentField
                width: parent.width
                label: "User-Agent"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                onTextChanged: settings.userAgent = text
                Keys.onReturnPressed: focus = false
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

            Item { width: 1; height: AppTheme.paddingLarge }
        }
        VerticalScrollDecorator { }
    }
}
