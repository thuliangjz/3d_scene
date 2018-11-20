#ifndef SCENE_GUI
#define SCENE_GUI
#include <QtQuick/QQuickItem>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include "scene_render_dynamic.h"
#include "geometry.h"
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
    void processTriNode(aiNode* node, const aiScene* scene);
    TriangleMesh processTriMesh(aiMesh* mesh, const aiScene* scene);
private:
    SceneRenderDynamic *m_render_dynamic;
    vector<TriangleMesh> m_triangle_meshes; //三角形网格用于传递给render进行绘制
    bool m_scene_reloaded;
};
#endif
