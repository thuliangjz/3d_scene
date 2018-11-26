#include "scene_render_dynamic.h"
#include "scene_gui.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <QDebug>

SceneRenderDynamic::SceneRenderDynamic():
    m_window(nullptr), m_program(nullptr),
    m_vbo(nullptr), m_ebo(nullptr), m_vao(nullptr),
    m_program_shadow(nullptr), m_shadow_fbo(nullptr), size_shadow(1024, 1024),
    m_gl_context_inited(false), m_mesh_reloaded(false),
    m_magnitude(1.f), m_shadow_valid(false){
}

SceneRenderDynamic::~SceneRenderDynamic(){
    delete m_vao;
    delete m_vbo;
    delete m_ebo;
    delete m_program;
}

void SceneRenderDynamic::setCamera(const glm::vec3 &pos, const glm::vec3& dir){
    assert(glm::cross(y_axis, dir) != glm::vec3(0, 0, 0));      //约定相机方向不能指向正上方（否则无法指定up）
    m_camera_dir = dir;
    m_camera_pos = pos;
}

void SceneRenderDynamic::setMesh(const vector<TriangleMesh>& meshes, SceneGUI *gui){
    m_mesh_reloaded = true;
    m_meshes = meshes;
    m_bias = gui->m_bias;
    map<aiTextureType, uint> m;
    m[aiTextureType_AMBIENT] = TYPE_UNIT_AMBIENT;
    m[aiTextureType_DIFFUSE] = TYPE_UNIT_DIFFUSE;
    m[aiTextureType_SPECULAR] = TYPE_UNIT_SPECULAR;
    for(int i = 0; i < m_meshes.size(); ++i){
        m_meshes[i].idx_textures.clear();
        for(auto &idx : meshes[i].idx_textures){
            FileTexture tex = gui->m_textures[idx];
            //将FileTexture中的type映射到texture unit
            RenderTexture tex_r;
            tex_r.id_sampler = m[tex.type];
            //如果存在QImage的分裂，则原先到img的映射也需要相应的进行修改
            tex_r.idx_texture_obj = tex.idx_image;
            m_meshes[i].idx_textures.push_back(m_textures.size());

            m_textures.push_back(tex_r);
        }
    }
    m_img_gui = gui->m_texture_imgs;
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
    m_count_triangles = idxs.size();
    m_vbo->create();
    m_vbo->bind();
    m_vbo->allocate(v.data(), sizeof(Vertex) * v.size());

    m_ebo->create();
    m_ebo->bind();
    m_ebo->allocate(idxs.data(), sizeof(int) * idxs.size());
    unbindObjects();
}

void SceneRenderDynamic::prepareTexture() {
    for(auto & pt_texture : m_texture_objs){
        delete  pt_texture;
    }
    for(auto & pt_img : m_img_gui){
        QOpenGLTexture *t = new QOpenGLTexture(*pt_img);
        t->setMagnificationFilter(QOpenGLTexture::Linear);
        t->setMagnificationFilter(QOpenGLTexture::Linear);
        t->setWrapMode(QOpenGLTexture::Repeat);
        m_texture_objs.push_back(t);
        delete pt_img;
    }
}

void SceneRenderDynamic::renderDepthMap(){
    m_shadow_fbo->bind();
    glViewport(0, 0, size_shadow.width(), size_shadow.height());
    glEnable(GL_DEPTH_TEST);

    glClearColor(1.f,1.f,1.f,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    m_program_shadow->bind();
    m_vao->bind();
    m_program_shadow->setUniformValue("light_transform", getLightTransform() * getModelMatrix());
    glDrawElements(GL_TRIANGLES, m_count_triangles, GL_UNSIGNED_INT, nullptr);
    m_program_shadow->release();
    m_vao->release();
    m_shadow_fbo->release();
//    m_shadow_fbo->toImage().save("shadow.png");
}

void SceneRenderDynamic::attachShadow(){
    uint tex_id = m_shadow_fbo->texture();
    glActiveTexture(GL_TEXTURE0 + TYPE_UNIT_SHADOW);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border_color[] = {1., 1., 1., 1.};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
    m_program->setUniformValue("texture_shadow", TYPE_UNIT_SHADOW);
}

void SceneRenderDynamic::useVertexData(){
    bindObjects();
    m_program->bind();
    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    m_program->enableAttributeArray(2);

    m_program->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, pos), 3, sizeof(Vertex));
    m_program->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));
    m_program->setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, tex_uv), 2, sizeof(Vertex));

    m_program_shadow->bind();
    m_program_shadow->enableAttributeArray("pos_vtx");
    m_program_shadow->release();
    unbindObjects();

}

QMatrix4x4 SceneRenderDynamic::getModelMatrix(){
    glm::mat4 model(m_magnitude);
    model[3][3] = 1.f;  //设置为1才是缩放
    return QMatrix4x4(&glm::transpose(model)[0][0]);
}

QMatrix4x4 SceneRenderDynamic::getLightTransform(){
    glm::mat4 view_light = glm::lookAt(
                m_light_pos,
                m_light_pos + m_light_dir,
                glm::vec3(0, 1, 0));
    return QMatrix4x4(&glm::transpose((m_light_type == LIGHT_PARALLEL ? m_light_proj_ortho : m_light_proj_pstv)
                                                                       * view_light)[0][0]);
}

void SceneRenderDynamic::useParameter(){
    glm::mat4 model(m_magnitude);
    model[3][3] = 1.f;  //设置为1才是缩放

    glm::vec3 up = glm::cross(m_camera_dir, glm::vec3(0, 1, 0));
    up = glm::normalize(glm::cross(up, m_camera_dir));
    glm::mat4 view = glm::lookAt(
                m_camera_pos,
                m_camera_pos + m_camera_dir,
                up);
    glm::mat4 projection = m_view_projection;
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
    m_program->setUniformValue("light_dir_parallel", QVector3D(m_light_dir.x,
                                                               m_light_dir.y,
                                                               m_light_dir.z));
    m_program->setUniformValue("light_pos", QVector3D(m_light_pos.x,
                                                      m_light_pos.y,
                                                      m_light_pos.z));
    m_program->setUniformValue("light_color", QVector3D(.5f, .5f, .5f) * 2);
    m_program->setUniformValue("view_pos", QVector3D(m_camera_pos.x, m_camera_pos.y, m_camera_pos.z));
    m_program->setUniformValue("light_transform", getLightTransform() * getModelMatrix());

    m_program->setUniformValue("bias", m_bias);
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
    using std::pair;
    typedef map<uint, pair<string, string>> map_id2name;
    map_id2name m;
    m[TYPE_UNIT_AMBIENT] = pair<string, string>("texture_ambient", "use_ambient_sample");
    m[TYPE_UNIT_DIFFUSE] = pair<string, string>("texture_diffuse", "use_diffuse_sample");
    m[TYPE_UNIT_SPECULAR] = pair<string, string>("texture_specular", "use_specular_sample");

    for(auto i : m_meshes[idx_mesh].idx_textures){
        m_program->setUniformValue(m[m_textures[i].id_sampler].first.c_str(), m_textures[i].id_sampler);
        m_texture_objs[m_textures[i].idx_texture_obj]->bind(m_textures[i].id_sampler);
        m_program->setUniformValue(m[m_textures[i].id_sampler].second.c_str(), 0.f);
        m.erase(m_textures[i].id_sampler);
    }
    for(auto& p : m){
        m_program->setUniformValue(p.second.second.c_str(), 1.f);
    }
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
    m_program->bindAttributeLocation("tex_uv", 2);
    m_program->link();

    m_program_shadow = new QOpenGLShaderProgram();
    m_program_shadow->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shadow.vert");
    m_program_shadow->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shadow.frag");
    m_program_shadow->link();

    m_shadow_fbo = new QOpenGLFramebufferObject(size_shadow, QOpenGLFramebufferObject::Depth);
}

void SceneRenderDynamic::paint(){
    if(!m_gl_context_inited){
            init();
    }
    if(m_mesh_reloaded){
        prepareVertexData();
        useVertexData();
        prepareTexture();
        renderDepthMap();
        m_mesh_reloaded = false;
        m_shadow_valid = true;
    }
    if(!m_shadow_valid){
        renderDepthMap();
        m_shadow_valid = true;
    }
    bindObjects();
    useParameter();
    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.3f,0.2f,0.1f,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    attachShadow();
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
