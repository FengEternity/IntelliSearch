import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "."

ListView {
    id: searchListView

    // 移除 anchors.fill: parent，让父布局管理尺寸
    
    model: ListModel {
        ListElement {
            name: "博查"
            icon: "qrc:/resources/icons/model/bocha.ico"
        }

        ListElement {
            name: "Exa"
            icon: "qrc:/resources/icons/model/exa.png"
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

        background: HoverBackground {
            isHovered: parent.hovered
        }
    }
}
