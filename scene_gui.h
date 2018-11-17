#ifndef SCENE_GUI
#define SCENE_GUI
#include <QtQuick/QQuickItem>
#include <string>
#include "scene_render_dynamic.h"
using std::string;
class SceneGUI:public QQuickItem{
    Q_OBJECT
public:
    SceneGUI();
    bool loadScene(const string name);
    Q_INVOKABLE void paint();
public slots:
    void sync();
    void cleanup();
private slots:
    void handleWindowChanged(QQuickWindow* win);
private:
    SceneRenderDynamic *m_render_dynamic;
};
#endif
