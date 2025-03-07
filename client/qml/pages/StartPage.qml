import QtQuick
import QtQuick.Layouts
import "../components"

Rectangle {
    id: root
    // 移除冲突的anchors属性，使用width和height替代
    width: parent.width
    height: parent.height
    color: applicationWindow.isDarkTheme ? "#121212" : "#ffffff"
    
    // 添加颜色过渡动画
    Behavior on color {
        ColorAnimation { duration: 200 }
    }

    signal switchToChatPage(string initialMessage)

    // 使用 RowLayout 作为最外层布局
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 主要内容区域
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            // 移除冲突的anchors属性
            // anchors.fill: parent
            // anchors.centerIn: parent
            // anchors.margins: 20
            Layout.margins: 20
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
                    color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#000000"
                    
                    // 添加颜色过渡动画
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }

                Text {
                    id: text2
                    text: qsTr("我可以帮助你搜索各种内容，请把你的任务交给我！")
                    font.pixelSize: 16
                    Layout.alignment: Qt.AlignHCenter
                    opacity: 0.65
                    color: applicationWindow.isDarkTheme ? "#FFFFFF" : "#000000"
                    
                    // 添加颜色过渡动画
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
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
