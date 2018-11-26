#ifndef SCENE_RENDER_DYNAMIC
#define SCENE_RENDER_DYNAMIC
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLTexture>
#include <QtQuick/QQuickWindow>
#include <QtGui/QOpenGLFramebufferObject>
#include "geometry.h"
#include <glm/mat4x4.hpp>
#define TYPE_UNIT_AMBIENT 0
#define TYPE_UNIT_DIFFUSE 1
#define TYPE_UNIT_SPECULAR 2
#define TYPE_UNIT_SHADOW 3

class SceneGUI;
class SceneRenderDynamic: public QObject, protected QOpenGLFunctions{
    Q_OBJECT
public:
    SceneRenderDynamic();
    ~SceneRenderDynamic();
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setWindow(QQuickWindow *window) { m_window = window; }
    void setMesh(const vector<TriangleMesh>& meshes, SceneGUI* gui);
    void setCamera(const glm::vec3& pos, const glm::vec3& dir);
    void setFov(float fov){m_fov = fov;}
    void setLightParam(int type, glm::vec3 pos, glm::vec3 dir){
        m_light_type = type;
        m_light_pos = pos;
        m_light_dir = dir;
        m_shadow_valid = false;
    }
    void setLightProj(int type, glm::mat4 mat){
        if(type == LIGHT_POINT)
            m_light_proj_pstv = mat;
        else
            m_light_proj_ortho = mat;
        m_shadow_valid = false;
    }
    void setViewProj(glm::mat4 mat){
        m_view_projection = mat;
    }
public slots:
    void paint();

private:
    struct RenderTexture{
        uint id_sampler;
        int idx_texture_obj;
    };

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
    void prepareTexture();
    void renderDepthMap();
    void attachShadow();
    QMatrix4x4 getModelMatrix();
    QMatrix4x4 getLightTransform();
private:
    QSize m_viewportSize;
    QQuickWindow *m_window;
    QOpenGLShaderProgram *m_program;
    QOpenGLBuffer *m_vbo, *m_ebo;
    QOpenGLVertexArrayObject *m_vao;

    QOpenGLShaderProgram *m_program_shadow;
    QOpenGLFramebufferObject *m_shadow_fbo;
    const QSize size_shadow;

    vector<RenderTexture> m_textures;
    vector<QOpenGLTexture*> m_texture_objs;
    vector<QImage*> m_img_gui;

    bool m_gl_context_inited;
    int m_shader_program_id;
    glm::vec3 m_camera_pos;
    glm::vec3 m_camera_dir;
    bool m_mesh_reloaded;
    vector<TriangleMesh> m_meshes;  //不同的mesh只是uniform的值不同
    int m_count_triangles;
    float m_fov;
    float m_magnitude;

    int m_light_type;
    glm::vec3 m_light_pos;  //m_light_pos和m_light_dir用于设置view矩阵
    glm::vec3 m_light_dir;
    glm::mat4 m_light_proj_ortho, m_light_proj_pstv;
    bool m_shadow_valid;

    glm::mat4 m_view_projection;
    float m_bias;
};
#endif
