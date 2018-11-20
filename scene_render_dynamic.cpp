#include "scene_render_dynamic.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <QDebug>

SceneRenderDynamic::SceneRenderDynamic():
    m_window(nullptr), m_program(nullptr),
    m_vbo(nullptr), m_ebo(nullptr), m_vao(nullptr),
    m_gl_context_inited(false), m_mesh_reloaded(false){
}

SceneRenderDynamic::~SceneRenderDynamic(){
    delete m_vao;
    delete m_vbo;
    delete m_ebo;
    delete m_program;
}

void SceneRenderDynamic::setCamera(const glm::vec3 &pos, const glm::vec3& dir){
    glm::vec3 y_axis = glm::vec3(0, 1, 0);
    assert(glm::cross(y_axis, dir) != glm::vec3(0, 0, 0));      //约定相机方向不能指向正上方（否则无法指定up）
    m_camera_dir = dir;
    m_camera_pos = pos;
}

void SceneRenderDynamic::setMesh(const vector<TriangleMesh>& meshes){
    m_mesh_reloaded = true;
    m_meshes = meshes;
}

void SceneRenderDynamic::prepareVertexData(){
    delete m_vao;
    delete m_vbo;
    delete m_ebo;

    m_vao = new QOpenGLVertexArrayObject();
    m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_ebo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

    m_vao->create();
    m_vao->bind();  //只有当bind的时候才能有效地将ebo的信息记录在vao中

    vector<Vertex> v;
    vector<int> idxs;
    //注意不同的mesh中index对应自己的顶点序号
    for(auto &mesh : m_meshes){
        vector<int> idx_tmp = mesh.indices;
        for(auto& i:idx_tmp){i += v.size();}
        idxs.insert(idxs.end(), idx_tmp.begin(), idx_tmp.end());
        v.insert(v.end(), mesh.vertices.begin(), mesh.vertices.end());
    }
    m_vbo->create();
    m_vbo->bind();
    m_vbo->allocate(v.data(), sizeof(Vertex) * v.size());

    m_ebo->create();
    m_ebo->bind();
    m_ebo->allocate(idxs.data(), sizeof(int) * idxs.size());
    unbindObjects();
}

void SceneRenderDynamic::useVertexData(){
    bindObjects();
    m_program->bind();
    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);

    m_program->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, pos), 3, sizeof(Vertex));
    m_program->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));

    unbindObjects();

}

void SceneRenderDynamic::useParameter(){
    glm::mat4 model(1.f);
    model[3][3] = 1.f;  //设置为1才是缩放

    glm::vec3 up = glm::cross(m_camera_dir, glm::vec3(0, 1, 0));
    up = glm::normalize(glm::cross(up, m_camera_dir));
    glm::mat4 view = glm::lookAt(
                m_camera_pos,
                m_camera_pos + m_camera_dir,
                up);

    glm::mat4 projection = glm::perspective(
                glm::radians(m_fov),
                static_cast<float>(m_viewportSize.width()) / m_viewportSize.height(),
                0.1f, 100.f);
    glm::mat3 normalization = glm::mat3(glm::transpose(glm::inverse(model)));
    model = glm::transpose(model); view = glm::transpose(view);
    projection = glm::transpose(projection); normalization = glm::transpose(normalization);
    QMatrix4x4 qmodel(&model[0][0]),
            qview(&view[0][0]),
            qprojection(&projection[0][0]);
    QMatrix3x3 qnormalization(&normalization[0][0]);

    m_program->setUniformValue("model", qmodel);
    m_program->setUniformValue("view", qview);
    m_program->setUniformValue("projection", qprojection);
    m_program->setUniformValue("normalization", qnormalization);
    m_program->setUniformValue("light_pos", QVector3D(0, 2, 0));
    m_program->setUniformValue("light_color", QVector3D(.5f, .5f, .5f) * 2);
    m_program->setUniformValue("view_pos", QVector3D(m_camera_pos.x, m_camera_pos.y, m_camera_pos.z));
}

void SceneRenderDynamic::useMeshData(int idx_mesh){
    m_program->setUniformValue("k_a", QVector3D(m_meshes[idx_mesh].k_a.r,
                                                m_meshes[idx_mesh].k_a.g,
                                                m_meshes[idx_mesh].k_a.b));
    m_program->setUniformValue("k_d", QVector3D(m_meshes[idx_mesh].k_d.r,
                                            m_meshes[idx_mesh].k_d.g,
                                            m_meshes[idx_mesh].k_d.b));
    m_program->setUniformValue("k_s", QVector3D(m_meshes[idx_mesh].k_s.r,
                                            m_meshes[idx_mesh].k_s.g,
                                            m_meshes[idx_mesh].k_s.b));

}

inline void SceneRenderDynamic::bindObjects(){
    m_program->bind();
    m_vao->bind();
    m_ebo->bind();
    m_vbo->bind();
}

inline void SceneRenderDynamic::unbindObjects(){
    m_program->release();
    m_vbo->release();
    m_vao->release();
    m_ebo->release();
}

void SceneRenderDynamic::init(){
    initializeOpenGLFunctions();
    m_gl_context_inited = true;
    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/phong.vert");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/phong.frag");
    m_program->bindAttributeLocation("pos_vtx", 0);
    m_program->bindAttributeLocation("normal_vtx", 1);
    m_program->link();
}

void SceneRenderDynamic::paint(){
    if(!m_gl_context_inited){
            init();
    }
    if(m_mesh_reloaded){
        prepareVertexData();
        useVertexData();
        m_mesh_reloaded = false;
    }
    bindObjects();
    useParameter();
    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
    glEnable(GL_DEPTH_TEST);

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);
    int *start = nullptr;
    int idx_mesh = 0;
    for(auto &mesh : m_meshes){
        useMeshData(idx_mesh);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, start);
        start += mesh.indices.size();
        ++idx_mesh;
    }
    unbindObjects();
    m_window->resetOpenGLState();
}
