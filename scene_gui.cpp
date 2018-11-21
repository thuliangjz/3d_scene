#include "scene_gui.h"
#include <QtQuick/qquickwindow.h>
#include <QDebug>
#include <unistd.h>
#include <glm/glm.hpp>
#include <QGuiApplication>
SceneGUI::SceneGUI(): m_render_dynamic(nullptr), m_scene_reloaded(false),
    m_cursor_hidden(false){
    connect(this, &QQuickItem::windowChanged, this, &SceneGUI::handleWindowChanged);
    m_obj_dir = "windmill/";
    if(!loadScene(m_obj_dir + "Windmill.obj")){
        exit(-1);
    }

    m_camera_pos = glm::vec3(0.f, 0.f, 10.f);
    m_camera_dir = -glm::normalize(m_camera_pos);
    m_epsilon_move = 1e-5f;
    m_delta_move = .5f;
    m_delta_rotate_h = glm::radians(3.f);
    m_delta_rotate_v = glm::radians(.2f);
    max_pitch = glm::radians(89.f);
    m_fov = 45.f;
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
    m_camera_pos += mv_delta;
    glm::vec3 pos_old = m_camera_pos;
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
    }
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
    for(int i = 0; i < mesh->mNumVertices; ++i){
        Vertex v;
        v.pos.x = mesh->mVertices[i].x;
        v.pos.y = mesh->mVertices[i].y;
        v.pos.z = mesh->mVertices[i].z;
        v.normal.x = mesh->mNormals[i].x;
        v.normal.y = mesh->mNormals[i].y;
        v.normal.z = mesh->mNormals[i].z;
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
