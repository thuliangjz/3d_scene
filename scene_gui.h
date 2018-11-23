#ifndef SCENE_GUI
#define SCENE_GUI
#include <QtQuick/QQuickItem>
#include <QVector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <map>
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
using std::map;
class SceneGUI: public QQuickItem{
    Q_OBJECT
public:
    SceneGUI();
    bool loadScene(const string name);
    Q_INVOKABLE void paint();
    Q_INVOKABLE bool updateCameraPos(int move_state);
    Q_INVOKABLE bool updateCameraDir(qreal d_x, qreal d_y);
    Q_INVOKABLE void zoom(qreal angle_delta);
    Q_INVOKABLE QVector3D getDirection(){return QVector3D(m_camera_dir.x,
                                                          m_camera_dir.y,
                                                          m_camera_dir.z);}
    Q_INVOKABLE QVector3D getPosition(){return QVector3D(m_camera_pos.x,
                                                    m_camera_pos.y,
                                                    m_camera_pos.z);}
public:
    struct FileTexture{
        aiTextureType type;
        int idx_image;
    };

public slots:
    void sync();
    void cleanup();
private slots:
    void handleWindowChanged(QQuickWindow* win);
private:
    void processTriNode(aiNode* node, const aiScene* scene);
    TriangleMesh processTriMesh(aiMesh* mesh, const aiScene* scene);
    vector<int> loadMeshTexture(aiMaterial* material, aiTextureType type);      //返回该mesh的纹理编号
    bool collisionCheck();
    friend void SceneRenderDynamic::setMesh(const vector<TriangleMesh>& meshes, SceneGUI *gui);
private:
    SceneRenderDynamic *m_render_dynamic;
    vector<TriangleMesh> m_triangle_meshes; //三角形网格用于传递给render进行绘制
    vector<FileTexture> m_textures;
    vector<QImage*> m_texture_imgs;
    map<string, int> m_map_name2img;

    bool m_scene_reloaded;
    glm::vec3 m_camera_pos;
    glm::vec3 m_camera_dir;

    float m_delta_move;
    float m_epsilon_move;
    float m_delta_rotate_h;
    float m_delta_rotate_v;
    float m_delta_fov;
    float m_fov;
    float max_pitch;
    bool m_cursor_hidden;

    string m_obj_dir;
};
#endif
