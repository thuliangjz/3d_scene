#include "scene_gui.h"
#include <QtQuick/qquickwindow.h>
#include <QDebug>
#include <unistd.h>
SceneGUI::SceneGUI(): m_render_dynamic(nullptr), m_scene_reloaded(false){
    connect(this, &QQuickItem::windowChanged, this, &SceneGUI::handleWindowChanged);
    if(!loadScene("airboat.obj")){
        exit(-1);
    }
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
    if(m_scene_reloaded){
        m_scene_reloaded = false;
        m_render_dynamic->setMesh(m_triangle_meshes);
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
        vertices.push_back(v);
    }
    for(int i = 0; i < mesh->mNumFaces; ++i){
        for(int j = 0; j < 3; ++j){
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }
    return TriangleMesh(vertices, indices);
}
