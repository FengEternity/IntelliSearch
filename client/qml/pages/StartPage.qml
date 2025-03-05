import QtQuick
import QtQuick.Layouts
import "../components"

Rectangle {
    id: root
    anchors.fill: parent
    anchors.centerIn: parent

    signal switchToChatPage(string initialMessage)

    // 使用 RowLayout 作为最外层布局
    RowLayout {
        anchors.fill: parent
        spacing: 0


        // 主要内容区域
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            anchors.fill: parent
            anchors.centerIn: parent
            anchors.margins: 20
            spacing: 40
            
            // 顶部留白
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
            }

            Image {
                id: logo
                source: "qrc:/resources/icons/logo.svg"
                fillMode: Image.PreserveAspectFit
                opacity: 0.8
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(389, parent.width * 0.3)
                Layout.preferredHeight: Math.min(255, parent.width * 0.2)
            }

            // 文字部分
            ColumnLayout {
                id: columnLayout
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                Text {
                    id: text1
                    text: qsTr("欢迎使用 IntelliSearch")
                    font.pixelSize: 40
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    id: text2
                    text: qsTr("我可以帮助你搜索各种内容，请把你的任务交给我！")
                    font.pixelSize: 16
                    Layout.alignment: Qt.AlignHCenter
                    opacity: 0.65
                }
            }

            // 输入框
            ChatTextField {
                id: chatInput
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(426, parent.width * 0.8)
                Layout.topMargin: 20
                
                onTextSubmitted: function(text) {
                    root.switchToChatPage(text)
                }
            }

            // 底部留白
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
