import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.4
import MyScene 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 640
    height: 480
    title: qsTr("3d场景漫游")
    SceneGUI{
        id: scene
        anchors.fill: parent
        Navagator{
            id: navagator
            focus: true
            anchors.fill: parent
        }
    }
    MouseArea{
        id: ma_window
        x:0
        y:0
        width: 100
        height: 100
        onClicked: {
            navagator.navagating = !navagator.navagating
        }
    }

    Component.onCompleted: {
        navagator.Keys.pressed.connect(navagator.navKeyPressHandler(scene))
        navagator.navMousePosChanged.connect(navagator.navMouseMovHandler(scene))
        navagator.navMouseWhl.connect(navagator.navMouseWhlHandler(scene))
    }
}
