#ifndef SCENE_GUI
#define SCENE_GUI
#include <QtQuick/QQuickItem>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include "scene_render_dynamic.h"
#include "geometry.h"
#include <QCursor>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#define GUI_MV_STATE_STAY 0
#define GUI_MV_STATE_LEFT 1
#define GUI_MV_STATE_RIGHT 2
#define GUI_MV_STATE_FRONT 3
#define GUI_MV_STATE_BACK 4
#define GUI_MV_STATE_UP 5
#define GUI_MV_STATE_DOWN 6
using std::string;
class SceneGUI:public QQuickItem{
    Q_OBJECT
public:
    SceneGUI();
    bool loadScene(const string name);
    Q_INVOKABLE void paint();
    Q_INVOKABLE bool updateCameraPos(int move_state);
    Q_INVOKABLE bool updateCameraDir(qreal d_x, qreal d_y);
public slots:
    void sync();
    void cleanup();
private slots:
    void handleWindowChanged(QQuickWindow* win);
private:
    void processTriNode(aiNode* node, const aiScene* scene);
    TriangleMesh processTriMesh(aiMesh* mesh, const aiScene* scene);
    bool collisionCheck();
private:
    SceneRenderDynamic *m_render_dynamic;
    vector<TriangleMesh> m_triangle_meshes; //三角形网格用于传递给render进行绘制
    bool m_scene_reloaded;
    glm::vec3 m_camera_pos;
    glm::vec3 m_camera_dir;

    float m_delta_move;
    float m_epsilon_move;
    float m_delta_rotate_h;
    float m_delta_rotate_v;
    float max_pitch;
    bool m_cursor_hidden;
    QWidget *m_mouse_area;
};
#endif
