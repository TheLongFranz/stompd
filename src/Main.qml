import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: true
    width: 800
    height: 480
    title: "hello-world"

    Column {
        anchors.centerIn: parent
        spacing: 32

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "hello-world"
            font.pixelSize: 48
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Close"
            font.pixelSize: 24
            onClicked: Qt.quit()
        }
    }
}
