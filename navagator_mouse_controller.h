#ifndef NAVAGATOR_MOUSE_CONTROLLER_H
#define NAVAGATOR_MOUSE_CONTROLLER_H

#include <QQuickItem>

class NavagatorMouseController : public QQuickItem
{
    Q_OBJECT
public:
    NavagatorMouseController();
    Q_INVOKABLE void resetMousePos();
    Q_INVOKABLE void hideCursor();
    Q_INVOKABLE void showCursor();
    Q_INVOKABLE QPointF getDelta(qreal x, qreal y);
private:
    bool m_cursor_hidden;
};

#endif // NAVAGATOR_MOUSE_CONTROLLER_H
