#ifndef RAYT_RACER_H
#define RAYT_RACER_H

#include <QObject>
#include <QImage>
#include "geometry.h"

struct TraceParam{
    vector<TriangleMesh> meshes;
    glm::vec3 camera_dir;
    glm::vec3 camera_pos;
    float fov;
    float z_near;
    float z_far;
    QSize sz_img;
    LightParam light;
};

class RayTracer : public QObject
{
    Q_OBJECT
public:
    explicit RayTracer(QObject *parent = nullptr);
    ~RayTracer();
signals:
    void traceFinished(QImage* result);
    void processUpdated(int process);
public slots:
    void work(void* pt);
private:
    glm::vec3 computeLightRec(glm::vec3 pos, glm::vec3 dir, glm::vec3 weight, int depth = 0);
    void getIntersectionBF(glm::vec3 pos, glm::vec3 dir, bool& success, float &t);
private:
    vector<TriangleMesh> m_meshes;
    LightParam m_light;
    glm::vec3 m_view_pos;
    float m_max_ks;
};

#endif // RAYT_RACER_H
