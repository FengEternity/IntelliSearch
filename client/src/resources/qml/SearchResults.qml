import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

ScrollView {
    id: searchResultsView
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.maximumWidth: 800
    Layout.alignment: Qt.AlignHCenter
    clip: true
    
    property alias model: resultsList.model
    
    ListView {
        id: resultsList
        width: parent.width
        model: ListModel {}
        delegate: ItemDelegate {
            id: resultDelegate
            width: parent.width
            height: 80
            padding: 10
            
            background: Rectangle {
                color: resultDelegate.hovered ? (Material.theme === Material.Dark ? Qt.darker(Material.background, 1.2) : Qt.lighter(Material.background, 1.2)) : "transparent"
                Behavior on color { ColorAnimation { duration: 150 } }
            }
            
            RowLayout {
                anchors.fill: parent
                spacing: 10
                
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    
                    Text {
                        Layout.fillWidth: true
                        text: model.title || ""
                        font.pixelSize: 16
                        font.weight: Font.Medium
                        color: Material.primary
                        elide: Text.ElideRight
                    }
                    
                    Text {
                        Layout.fillWidth: true
                        text: model.description || ""
                        font.pixelSize: 14
                        color: Material.foreground
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                }
                
                Rectangle {
                    Layout.preferredWidth: 4
                    Layout.fillHeight: true
                    color: Material.accent
                    opacity: 0.7
                    radius: 2
                }
            }
        }
    }
}