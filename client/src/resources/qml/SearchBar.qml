import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: searchBar
    Layout.fillWidth: true
    Layout.preferredHeight: 60
    Layout.maximumWidth: 800
    radius: height / 2
    color: "#f5f5f5"
    border.color: searchField.activeFocus ? Material.accentColor : "transparent"
    border.width: 2
    
    property alias text: searchField.text
    signal search(string query)
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        TextField {
            id: searchField
            Layout.fillWidth: true
            Layout.fillHeight: true
            placeholderText: "输入搜索内容..."
            background: null
            selectByMouse: true
            font.pixelSize: 16
            verticalAlignment: TextInput.AlignVCenter
            
            Keys.onReturnPressed: {
                if (text.length > 0) {
                    searchBar.search(text)
                }
            }
        }
        
        RoundButton {
            Layout.preferredWidth: 40
            Layout.preferredHeight: 40
            icon.source: "qrc:/icons/search.svg"
            icon.color: Material.foreground
            flat: true
            enabled: searchField.text.length > 0
            opacity: enabled ? 1.0 : 0.5
            
            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
            
            onClicked: {
                if (searchField.text.length > 0) {
                    searchBar.search(searchField.text)
                }
            }
        }
    }
}