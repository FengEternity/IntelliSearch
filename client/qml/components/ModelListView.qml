import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ListView {
    id: modelListView

    anchors.fill: parent

    model: ListModel {
        ListElement {
            name: "Kimi"
            icon: "qrc:/resources/icons/model/kimi.svg"
        }

        ListElement {
            name: "腾讯混元"
            icon: "qrc:/resources/icons/model/hunyuan.svg"
        }

        ListElement {
            name: "DeepSeek"
            icon: "qrc:/resources/icons/model/deepseek.svg"
        }

        ListElement {
            name: "通义千问"
            icon: "qrc:/resources/icons/model/qwen.svg"
        }
    }

    delegate: ItemDelegate {
        width: parent.width
        height: 60
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            spacing: 12

            Image {
                source: model.icon
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                fillMode: Image.PreserveAspectFit
            }

            Label {
                text: model.name
                font.pixelSize: 14
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}
