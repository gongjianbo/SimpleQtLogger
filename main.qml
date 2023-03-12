import QtQml 2.12
import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Window {
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("QML Logger")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6
        RowLayout {
            spacing: 6
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            ComboBox {
                id: log_level
                implicitWidth: 90
                implicitHeight: 30
                model: ["Debug", "Info", "Warning", "Critical"]
            }
            TextField {
                id: log_edit
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                implicitHeight: 30
                selectByMouse: true
                text: "QML日志信息"
            }
            Button {
                id: log_enter
                implicitWidth: 60
                implicitHeight: 30
                text: "Log"
                onClicked: {
                    // 没有 console.fatal
                    switch (log_level.currentIndex)
                    {
                    case 0: console.debug(log_edit.text); break;
                    case 1: console.info(log_edit.text); break;
                    case 2: console.warn(log_edit.text); break;
                    case 3: console.error(log_edit.text); break;
                    }
                }
            }
        }
        TextArea {
            id: log_view
            Layout.fillWidth: true
            Layout.fillHeight: true
            wrapMode: TextArea.WordWrap
            textFormat: TextArea.RichText
            selectByMouse: true
            background: Rectangle {
                border.color: "gray"
            }
        }
    }

    Connections {
        target: logManager
        // 低版本的连接方式
        //onNewLog: {
        //    log_view.append(logManager.richText(msgType, log))
        //}
        function onNewLog(msgType, log)
        {
            log_view.append(logManager.richText(msgType, log))
        }
    }
}
