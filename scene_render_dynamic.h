#ifndef SCENE_RENDER_DYNAMIC
#define SCENE_RENDER_DYNAMIC
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtQuick/QQuickWindow>
#include "geometry.h"
class SceneRenderDynamic: public QObject, protected QOpenGLFunctions{
    Q_OBJECT
public:
    SceneRenderDynamic();
    ~SceneRenderDynamic();
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setWindow(QQuickWindow *window) { m_window = window; }
    void setMesh(const vector<TriangleMesh>& meshes);
    void setCamera(const glm::vec3& pos, const glm::vec3& dir);
public slots:
    void paint();
private:
    void init();
    void glInit();
    void prepareVertexData();
    //不同的程序相应使用顶点和绘制参数的方法可以不同
    void useVertexData();   //在vao中设置对顶点数据的使用方法
    void useParameter();    //在程序中设置绘制参数
    void useMeshData(int idx_mesh); //不同的mesh中诸如纹理图片或者材料等uniform信息不同，在该函数中进行设置
    inline void bindObjects();
    inline void unbindObjects();
private:
    class MashMataData {
    public:
        int m_idx_start;
        int m_cnt;
    };
    QSize m_viewportSize;
    QQuickWindow *m_window;
    QOpenGLShaderProgram *m_program;
    QOpenGLBuffer *m_vbo, *m_ebo;
    QOpenGLVertexArrayObject *m_vao;
    bool m_gl_context_inited;
    int m_shader_program_id;
    glm::vec3 m_camera_pos;
    glm::vec3 m_camera_dir;
    bool m_mesh_reloaded;
    vector<TriangleMesh> m_meshes;  //不同的mesh只是uniform的值不同
};
#endif
