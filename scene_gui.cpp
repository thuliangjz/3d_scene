#include "scene_gui.h"
#include <QtQuick/qquickwindow.h>
#include <QDebug>
#include <unistd.h>
#include <glm/glm.hpp>
#include <QGuiApplication>
#include "utils.h"
SceneGUI::SceneGUI(): m_render_dynamic(nullptr), m_scene_reloaded(false),
    m_cursor_hidden(false), m_tracer_working(false){
    initFromCmd();
    connect(this, &QQuickItem::windowChanged, this, &SceneGUI::handleWindowChanged);

//    m_obj_dir = "./";
//    m_obj_name = "simple_scene.obj";
//    m_camera_pos = glm::vec3(8.94211, 5.5, -0.641418);
//    m_camera_dir = glm::vec3(-0.951188, -0.293211, -0.0962711);
//    m_bias = 0.005f;
//    m_flip_normal = true;
//    m_light_type = LIGHT_PARALLEL;
//    m_light_pos = glm::vec3(6.9383, 4.5, -1.58225);
//    m_light_dir = glm::vec3(-0.981657, -0.1906, 0.00459649);
//    m_light_mat = glm::ortho(-10.f, 10.f, -10.f, 10.f, 1.f, 590.f);

    m_epsilon_move = 1e-5f;
    m_delta_move = .5f;
    m_sz_camera = 1.f;
    m_delta_rotate_h = glm::radians(3.f);
    m_delta_rotate_v = glm::radians(.2f);
    max_pitch = glm::radians(89.f);
    m_fov = 45.f;
    m_z_near = .1f;
    m_z_far = 300.f;
    m_ray_tracer = new RayTracer();
    m_ray_tracer->moveToThread(&m_thread_tracer);
    connect(&m_thread_tracer, &QThread::finished, m_ray_tracer, &QObject::deleteLater);
    connect(this, &SceneGUI::tracerStart, m_ray_tracer, &RayTracer::work);
    connect(m_ray_tracer, &RayTracer::traceFinished, this, &SceneGUI::tracerFinished);
    connect(m_ray_tracer, &RayTracer::processUpdated, this, &SceneGUI::tracerProcessUpdated);
    m_thread_tracer.start();

    if(!loadScene(m_obj_dir + m_obj_name)){
        exit(-1);
    }
}

void SceneGUI::zoom(qreal angle_delta){
    m_fov += static_cast<float>(-angle_delta) * .01f;
    const float max_fov = 89, min_fov = 5;
    m_fov = m_fov > max_fov ? max_fov : m_fov;
    m_fov = m_fov < min_fov ? min_fov : m_fov;
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

bool SceneGUI::updateCameraPos(int move_state){
    glm::vec3 projection_xz = glm::vec3(m_camera_dir.x, 0, m_camera_dir.z);
    if(glm::all(glm::lessThan(glm::abs(projection_xz), glm::vec3(m_epsilon_move))))
        projection_xz = glm::vec3(0.f);
    else
        projection_xz = glm::normalize(projection_xz);
    glm::vec3 proj_left = glm::transpose(glm::mat3(
                0, 0, 1,
                0, 1, 0,
                -1, 0, 0
                )) * projection_xz;
    glm::vec3 mv_delta(0, 1, 0);
    switch(move_state){
    case GUI_MV_STATE_FRONT:
        mv_delta = projection_xz * m_delta_move;
        break;
    case GUI_MV_STATE_BACK:
        mv_delta = projection_xz * -m_delta_move;
        break;
    case GUI_MV_STATE_LEFT:
        mv_delta = proj_left * m_delta_move;
        break;
    case GUI_MV_STATE_RIGHT:
        mv_delta = proj_left * -m_delta_move;
        break;
    case GUI_MV_STATE_UP:
        mv_delta = mv_delta * m_delta_move;
        break;
    case GUI_MV_STATE_DOWN:
        mv_delta = mv_delta * -m_delta_move;
        break;
    default:
        mv_delta = glm::vec3(0.f,0.f,0.f);
        break;
    }
    glm::vec3 pos_old = m_camera_pos;
    m_camera_pos += mv_delta;
    if(collisionCheck()){
        m_camera_pos = pos_old;
    }
    paint();
    return true;
}

bool SceneGUI::updateCameraDir(qreal d_x, qreal d_y){
    //水平旋转
    m_camera_dir = glm::rotate(m_camera_dir, glm::radians(static_cast<float>(-d_x) * m_delta_rotate_h), glm::vec3(0, 1, 0));
    //在竖直方向旋转应该保证dir始终不垂直于xz平面
    glm::vec3 proj_xz = glm::normalize(glm::vec3(m_camera_dir.x, 0, m_camera_dir.z));
    float pitch = glm::angle(glm::normalize(m_camera_dir), proj_xz) * (m_camera_dir.y > 0 ? 1.f : -1.f);
    pitch += static_cast<float>(-d_y) * m_delta_rotate_v;
    pitch = pitch > max_pitch ? max_pitch : pitch;
    pitch = pitch < -max_pitch ? -max_pitch : pitch;
    m_camera_dir = (std::cos(pitch) * proj_xz) + (std::sin(pitch) * glm::vec3(0, 1, 0));
    paint();
    return true;
}

bool SceneGUI::collisionCheck(){
    float sz = 0.5f;
    glm::vec3 v0 = m_camera_pos - sz * glm::vec3(1,1,1);
    glm::vec3 e1 = sz * glm::vec3(1, 0, 0),
            e2 = sz * glm::vec3(0, 1, 0),
            e3 = sz * glm::vec3(0, 0, 1);
    for(auto &box : m_bounding_boxes){
        for(int i = 0; i < 8; ++i){
            float t1 = (i & 1) ? 0 : 1;
            float t2 = (i & 2) ? 0 : 1;
            float t3 = (i & 4) ? 0 : 1;
            glm::vec3 v_c = v0 + t1 * e1 + t2 * e2 + t3 * e3;
            if((glm::all(glm::lessThan(v_c, box.second))) &&
                 glm::all(glm::greaterThan(v_c, box.first)))
                return true;
        }
    }
    return false;
}

void SceneGUI::paint(){
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
    m_render_dynamic->setCamera(m_camera_pos, m_camera_dir);
    m_render_dynamic->setFov(m_fov);
    if(m_scene_reloaded){
        m_scene_reloaded = false;
        m_render_dynamic->setMesh(m_triangle_meshes, this);
        setRenderLight();
    }
    setRenderViewProj();
}

bool SceneGUI::loadScene(const string name){
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(name, aiProcess_Triangulate | aiProcess_GenNormals);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
        qDebug() << "error when reading model file: " << importer.GetErrorString();
        return false;
    }
    m_triangle_meshes.clear();
    //注意在loadScene中只负责清空，指针指向对象真正的删除在render对象中(当render线程实现了将代码拷贝到gui线程之后)
    m_texture_imgs.clear();
    m_textures.clear();
    m_map_name2img.clear();
    processTriNode(scene->mRootNode, scene);
    //注意scene随着importer的析构而失效
    m_scene_reloaded = true;
    generateBoundingBxs();
    return true;
}

void SceneGUI::processTriNode(aiNode* node, const aiScene* scene){
    for(int i = 0; i < node->mNumMeshes; ++i){
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        m_triangle_meshes.push_back(processTriMesh(mesh, scene));
    }
    for(int i = 0; i < node->mNumChildren; ++i){
        processTriNode(node->mChildren[i], scene);
    }
}

TriangleMesh SceneGUI::processTriMesh(aiMesh* mesh, const aiScene* scene){
    vector<int> indices;
    vector<Vertex> vertices;
    float sgn = m_flip_normal ? -1 : 1;
    for(int i = 0; i < mesh->mNumVertices; ++i){
        Vertex v;
        v.pos.x = mesh->mVertices[i].x;
        v.pos.y = mesh->mVertices[i].y;
        v.pos.z = mesh->mVertices[i].z;
        v.normal.x = sgn * mesh->mNormals[i].x;
        v.normal.y = sgn * mesh->mNormals[i].y;
        v.normal.z = sgn * mesh->mNormals[i].z;
        if(mesh->mTextureCoords[0]){
            v.tex_uv.x = mesh->mTextureCoords[0][i].x;
            v.tex_uv.y = mesh->mTextureCoords[0][i].y;
        }
        vertices.push_back(v);
    }
    for(int i = 0; i < mesh->mNumFaces; ++i){
        for(int j = 0; j < 3; ++j){
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }
    //设置表面参数
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    aiColor3D color_disfuse(0, 0, 0), color_spec(0, 0, 0), color_amb(0, 0, 0);
    material->Get(AI_MATKEY_COLOR_AMBIENT, color_amb);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, color_disfuse);
    material->Get(AI_MATKEY_COLOR_SPECULAR, color_spec);
    //加载纹理，当前支持的纹理包括环境，漫反射和镜面反射纹理
    aiTextureType type_textures[] = {aiTextureType_AMBIENT, aiTextureType_DIFFUSE, aiTextureType_SPECULAR};
    vector<int> idx_textures;
    for(int i = 0; i < sizeof(type_textures) / sizeof(aiTextureType); ++i){
        vector<int> idxs_tmp = loadMeshTexture(material, type_textures[i]);
        idx_textures.insert(idx_textures.end(), idxs_tmp.begin(), idxs_tmp.end());
    }
    return TriangleMesh(vertices, indices, aiColor3Toglm3(color_amb), aiColor3Toglm3(color_disfuse), aiColor3Toglm3(color_spec), idx_textures);
}

vector<int> SceneGUI::loadMeshTexture(aiMaterial* material, aiTextureType type){
    vector<int> result;
    for(int i = 0; i < material->GetTextureCount(type); ++i){
        aiString astr_name;
        material->GetTexture(type, i, &astr_name);
        string name(astr_name.C_Str());
        struct FileTexture texture_tmp;
        texture_tmp.type = type;
        if(m_map_name2img.find(name) == m_map_name2img.end()){
            m_map_name2img[name] = m_texture_imgs.size();
            QImage *img = new QImage((m_obj_dir + name).c_str());
            if(!img->isNull())
                m_texture_imgs.push_back(img);
        }
        texture_tmp.idx_image = m_map_name2img[name];
        result.push_back(m_textures.size());
        m_textures.push_back(texture_tmp);
    }
    return result;
}

bool SceneGUI::startRayTracing(){
    if(m_tracer_working)
        return false;
    TraceParam* p_param = new TraceParam;
    p_param->meshes = m_triangle_meshes;
    p_param->camera_dir = m_camera_dir; p_param->camera_pos = m_camera_pos;
    p_param->fov = m_fov; p_param->z_near = m_z_near; p_param->z_far = m_z_far;
    p_param->sz_img = QSize(window()->size() * window()->devicePixelRatio());
    p_param->light.color = glm::vec3(1, 1, 1);
    p_param->light.dir = m_light_type == LIGHT_POINT ? m_light_pos : m_light_dir;
    tracerStart(static_cast<void*>(p_param));
    m_tracer_working = true;
    return true;
}

void SceneGUI::tracerFinished(QImage* result){
    result->save("ray_tracing.png");
    qDebug() << "finished";
    m_tracer_working = false;
}

void SceneGUI::tracerProcessUpdated(int process){
    QSize s(window()->size() * window()->devicePixelRatio());
    qDebug() << process << "/" << s.width() * s.height();
}

void SceneGUI::setRenderLight(){
    m_render_dynamic->setLightParam(m_light_type,
                                    m_light_pos,
                                    m_light_dir);
    m_render_dynamic->setLightProj(m_light_type, m_light_mat);
}

void SceneGUI::setRenderViewProj(){
    QSizeF sz = window()->size() * window()->devicePixelRatio();
    glm::mat4 projection = glm::perspective(glm::radians(m_fov),
                                            static_cast<float>(sz.width() / sz.height()),
                                            m_z_near, m_z_far);
//    float h = 50.f;
//    glm::mat4 projection = glm::ortho(-h, h, -h, h, .1f, 300.f);
    m_render_dynamic->setViewProj(projection);
}

void SceneGUI::initFromCmd(){
    if(cmd_cnt < CMD_COUNT){
        qDebug() << "please use provided shell script to start";
        exit(-1);
    }
    m_obj_dir = cmd_args[CMD_OBJ_DIR];
    m_obj_name = cmd_args[CMD_OBJ_NAME];
    m_camera_pos = getVec3FromString(cmd_args[CMD_CAMERA_POS]);
    m_camera_dir = getVec3FromString(cmd_args[CMD_CAMERA_DIR]);
    m_bias = QString(cmd_args[CMD_BIAS]).toFloat();
    m_flip_normal = QString(cmd_args[CMD_FLIP_NORMAL]) == "true";
    m_light_type = QString(cmd_args[CMD_LIGHT_TYPE]) == "point" ? LIGHT_POINT : LIGHT_PARALLEL;
    m_light_pos = getVec3FromString(cmd_args[CMD_LIGHT_POS]);
    m_light_dir = getVec3FromString(cmd_args[CMD_LIGHT_DIR]);
    vector<float> v = getFloatsFromString(cmd_args[CMD_LIGHT_MAT]);
    if(v.size() != 3){
        qDebug() << "error setting light type";
        exit(-1);
    }

    if(m_light_type == LIGHT_PARALLEL){
        m_light_mat = glm::ortho(-v[0], v[0], -v[0], v[0], v[1], v[2]);
    }
    else{
        m_light_mat = glm::perspective(v[0], 1.f, v[1], v[2]);
    }
}

glm::vec3 SceneGUI::getVec3FromString(const char* str){
    vector<float> v = getFloatsFromString(str);
    return glm::vec3(v[0], v[1], v[2]);
}

vector<float> SceneGUI::getFloatsFromString(const char* str){
    vector<float> v;
    bool b;
    QStringList lst = QString(str).split(" ");
    for(auto &s : lst){
        float f = s.toFloat(&b);
        if(b)
            v.push_back(f);
    }
    return v;
}

void SceneGUI::generateBoundingBxs(){
    m_bounding_boxes.clear();
    for(auto &mesh : m_triangle_meshes){
        std::pair<glm::vec3, glm::vec3> box;
        for(int i = 0; i < 3; ++i){
            box.first.data.data[i] = mesh.vertices[0].pos.data.data[i];
            box.second.data.data[i] = mesh.vertices[0].pos.data.data[i];
            for(auto &v : mesh.vertices){
                box.first.data.data[i] = std::min(box.first.data.data[i], v.pos.data.data[i]);
                box.second.data.data[i] = std::max(box.second.data.data[i], v.pos.data.data[i]);
            }
        }
        m_bounding_boxes.push_back(box);
    }
}
