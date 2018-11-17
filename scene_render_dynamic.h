#ifndef SCENE_RENDER_DYNAMIC
#define SCENE_RENDER_DYNAMIC
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtQuick/QQuickWindow>
class SceneRenderDynamic: public QObject, protected QOpenGLFunctions{
    Q_OBJECT
public:
    SceneRenderDynamic();
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setWindow(QQuickWindow *window) { m_window = window; }

public slots:
    void paint();
private:
    void glInit();
private:
    QSize m_viewportSize;
    QQuickWindow *m_window;
    QOpenGLShaderProgram *m_program;
    QOpenGLBuffer *m_vbo, *m_ebo;
    QOpenGLVertexArrayObject *m_vao;
    bool m_gl_context_inited;
    int m_shader_program_id;
};
#endif
