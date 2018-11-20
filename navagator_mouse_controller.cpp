#include "navagator_mouse_controller.h"
#include <QCursor>
#include <QGuiApplication>
NavagatorMouseController::NavagatorMouseController():m_cursor_hidden(false)
{

}

void NavagatorMouseController::resetMousePos(){
    QPointF p = this->mapToGlobal(QPointF(this->width() / 2, this->height() / 2));
    QPoint pint(static_cast<int>(p.x()) ,static_cast<int>(p.y()));
    QCursor::setPos(pint);
}

void NavagatorMouseController::hideCursor(){
    if(m_cursor_hidden)
        return;
    m_cursor_hidden = true;
    QGuiApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
}

void NavagatorMouseController::showCursor(){
    if(!m_cursor_hidden)
        return;
    m_cursor_hidden = false;
    QGuiApplication::restoreOverrideCursor();
}

QPointF NavagatorMouseController::getDelta(qreal x, qreal y){
    QPointF res = QPointF(x - width() / 2, y - height() / 2);
    return res;
}
