import QtQuick 2.9
import MyScene 1.0
Item {
    id: navagator
    property bool navagating: false
    signal navMousePosChanged(real x, real y)
    signal navMouseWhl(real delta_y)
    function navKeyPressHandler(scene){
        return function(event){
            var type_mv = getMoveType(event.key)
            scene.updateCameraPos(type_mv)
            if(event.key === Qt.Key_Escape)
                navagating = false
            if(event.key === Qt.Key_J)
                console.log("direction: ",scene.getDirection())
            if(event.key === Qt.Key_K)
                console.log("position: ", scene.getPosition())
            if(event.key === Qt.Key_T)
                scene.startRayTracing()
            if(event.key === Qt.Key_N)
                navagating = !navagating
        }
    }
    function getMoveType(key){
        var type_mv = -1
        switch(key){
        case Qt.Key_A:
            type_mv = 1
            break;
        case Qt.Key_D:
            type_mv = 2
            break;
        case Qt.Key_W:
            type_mv = 3
            break;
        case Qt.Key_S:
            type_mv = 4
            break;
        case Qt.Key_Z:
            type_mv = 5
            break;
        case Qt.Key_X:
            type_mv = 6
            break;
        default:
            break;
        }
        return type_mv
    }
    function navMouseMovHandler(scene){
        return function(x, y){
            if(!navagating)
                return
            scene.updateCameraDir(x, y)
        }
    }
    function navMouseWhlHandler(scene){
        return function(delta_y){
            scene.zoom(delta_y)
            scene.paint()
        }
    }

    onNavagatingChanged: {
        if(navagating){
            mouse_controller.hideCursor()
            mouse_controller.resetMousePos()
        }
        else{
            //退出漫游
            mouse_controller.showCursor()
        }
    }
    NavMouseController{
        id:mouse_controller
        anchors.fill: parent
    }
    MouseArea {
        id:ma_navagator
        anchors.fill: parent
        hoverEnabled: true
        onPositionChanged: {
            if(navagating){
                var delta = mouse_controller.getDelta(mouse.x, mouse.y)
                navagator.navMousePosChanged(delta.x, delta.y)
                mouse_controller.resetMousePos()
            }
        }
        onWheel: {
            if(navagating)
                navagator.navMouseWhl(wheel.angleDelta.y)
        }
    }
}
