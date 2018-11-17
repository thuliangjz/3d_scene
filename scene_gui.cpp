#include "scene_gui.h"
#include <QtQuick/qquickwindow.h>
#include <QDebug>
SceneGUI::SceneGUI(): m_render_dynamic(nullptr){
    connect(this, &QQuickItem::windowChanged, this, &SceneGUI::handleWindowChanged);
}

void SceneGUI::handleWindowChanged(QQuickWindow *win){
    if(win){
        connect(win, &QQuickWindow::beforeSynchronizing, this, &SceneGUI::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &SceneGUI::cleanup, Qt::DirectConnection);
        win->setClearBeforeRendering(false);
    }
}

void SceneGUI::cleanup(){
    delete m_render_dynamic;
    m_render_dynamic = nullptr;
}

void SceneGUI::paint(){
    qDebug() << "paint called";
    if(window())
        window()->update();
}

void SceneGUI::sync(){
    if(!m_render_dynamic){
        m_render_dynamic = new SceneRenderDynamic();
        connect(window(), &QQuickWindow::beforeRendering, m_render_dynamic, &SceneRenderDynamic::paint, Qt::DirectConnection);
    }
    m_render_dynamic->setViewportSize(window()->size() * window()->devicePixelRatio());
    m_render_dynamic->setWindow(window());
}

bool SceneGUI::loadScene(const string name){
    return name == "";
}
