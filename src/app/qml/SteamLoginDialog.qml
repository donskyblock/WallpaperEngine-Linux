// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dlg
    title: qsTr("Sign in to Steam")
    modal: true
    anchors.centerIn: Overlay.overlay
    width: 420
    standardButtons: Dialog.NoButton
    property bool working: false
    property bool needGuard: false
    property string message: ""

    onOpened: { passField.text = ""; guardField.text = ""; needGuard = false; working = false; message = ""; userField.text = App.settings.steamUsername; userField.forceActiveFocus() }

    Connections {
        target: App
        function onGuardCodeRequired(kind) { dlg.needGuard = true; dlg.message = kind === "two-factor" ? qsTr("Enter the code from your authenticator app.") : qsTr("Enter the Steam Guard code sent to your email."); guardField.forceActiveFocus() }
        function onLoginFinished(ok, msg) { dlg.working = false; dlg.message = msg; if (ok) dlg.close() }
    }

    contentItem: ColumnLayout {
        spacing: 10
        Label {
            Layout.fillWidth: true; wrapMode: Text.Wrap; opacity: 0.75
            text: qsTr("Used once so steamcmd can cache a login for downloads. Your password goes straight to steamcmd and is never stored by this app.")
        }
        TextField { id: userField; Layout.fillWidth: true; placeholderText: qsTr("Steam username"); enabled: !dlg.working }
        TextField { id: passField; Layout.fillWidth: true; placeholderText: qsTr("Password"); echoMode: TextInput.Password; enabled: !dlg.working; onAccepted: signIn.clicked() }
        TextField {
            id: guardField
            Layout.fillWidth: true; visible: dlg.needGuard
            placeholderText: qsTr("Steam Guard code")
            onAccepted: { App.submitGuardCode(text); dlg.needGuard = false }
        }
        Label { visible: dlg.message !== ""; text: dlg.message; wrapMode: Text.Wrap; Layout.fillWidth: true; color: palette.text }
        ProgressBar { visible: dlg.working && !dlg.needGuard; indeterminate: true; Layout.fillWidth: true }
        RowLayout {
            Layout.alignment: Qt.AlignRight
            Button { text: qsTr("Cancel"); onClicked: dlg.close() }
            Button {
                id: signIn
                highlighted: true
                text: dlg.needGuard ? qsTr("Submit code") : qsTr("Sign in")
                enabled: dlg.needGuard ? guardField.text !== "" : (userField.text !== "" && passField.text !== "" && !dlg.working)
                onClicked: {
                    if (dlg.needGuard) { App.submitGuardCode(guardField.text); dlg.needGuard = false; return }
                    dlg.working = true; dlg.message = qsTr("Signing in…")
                    App.steamLogin(userField.text, passField.text)
                }
            }
        }
    }
}
