#include "ray_tracer.h"
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <QDebug>
RayTracer::RayTracer(QObject *parent) : QObject(parent){
}

RayTracer::~RayTracer(){
}

void RayTracer::work(void* pt){
    TraceParam* pt_param = static_cast<TraceParam*>(pt);
    const int report_interval = 10;
    m_meshes = pt_param->meshes;
    m_light = pt_param->light;
    m_view_pos = pt_param->camera_pos;
    glm::mat4 view = glm::lookAt(
                pt_param->camera_pos,
                pt_param->camera_dir + pt_param->camera_pos,
                glm::vec3(0, 1, 0));
    QSize sz_img = pt_param->sz_img;
    float z_far = pt_param->z_far, fov = pt_param->fov;
    delete  pt_param;
    m_max_ks = -1;
    for(auto &mesh : m_meshes){
        for(auto &v : mesh.vertices){
            v.pos = view * glm::vec4(v.pos, 1.f);
        }
        for(int i = 0; i < 3; ++i){
            float s = mesh.k_s.data.data[i];
            m_max_ks = m_max_ks > s ? m_max_ks : s;
        }
    }
    //计算左上角扫描坐标
    float ratio_wh =static_cast<float>(sz_img.width()) / sz_img.height();
    float top_left_y = z_far * glm::tan(fov / 2);
    glm::vec3 pos_top_left(-top_left_y * ratio_wh, top_left_y, -z_far);
    glm::vec3 pixel_delta(2 * top_left_y * ratio_wh / sz_img.width(),
                          -2* top_left_y / sz_img.height(), 0);
    pos_top_left += pixel_delta * 0.5f;
    QImage *result = new QImage(sz_img, QImage::Format_RGB32);
    glm::vec3 scan_dir = pos_top_left;
    int process = 0;
    if(m_max_ks >= 1 - 1e-3f){
        qDebug() << "error, k_s too big";
        return;
    }
//    glm::vec3 range(1 / (1 - m_max_ks) * 1.2f * m_light.color);
    glm::vec3 scan_dir_tmp = scan_dir;
    for(int i = 0; i < sz_img.width(); ++i){
        for(int j = 0; j < sz_img.height(); ++j){
            glm::vec3 color = computeLightRec(
                        glm::vec3(0, 0 ,0),
                        scan_dir_tmp, glm::vec3(1.f, 1, 1));
            int r = static_cast<int>(255 * color.r);
            int g = static_cast<int>(255 * color.g);
            int b = static_cast<int>(255 * color.b);
            r = r >= 0 ? r : 0; r = r <= 255 ? r : 255;
            g = g >= 0 ? g : 0; g = g <= 255 ? g : 255;
            b = b >= 0 ? b : 0; b = b <= 255 ? b : 255;
            result->setPixelColor(i, j, QColor(r, g, b));
            scan_dir_tmp.y += pixel_delta.y;
            ++process;
            if(process % report_interval == 0){
                processUpdated(process);
            }
        }
        scan_dir_tmp.y = scan_dir.y;
        scan_dir_tmp.x += pixel_delta.x;
    }
    scan_dir = scan_dir_tmp - pos_top_left;
    traceFinished(result);
}


glm::vec3 RayTracer::computeLightRec(glm::vec3 pos, glm::vec3 dir, glm::vec3 weight, int depth){
    const float threshold = 0.1f;
    if(glm::all(glm::lessThan(weight, glm::vec3(threshold, threshold, threshold))))
        return glm::vec3(0, 0, 0);
    bool intersect = false;
    int idx_mesh = 0, idx_face = 0;
    float t_min = -1;
    const float epsilon = 1e-8f;
    int i = 0;
    for(auto &mesh : m_meshes){
        for(int j = 0; j < mesh.indices.size(); j += 3){
            glm::vec3 p0 = mesh.vertices[mesh.indices[j]].pos,
                      p1 = mesh.vertices[mesh.indices[j + 1]].pos,
                      p2 = mesh.vertices[mesh.indices[j + 2]].pos;
            glm::vec3 e1 = p1 - p0, e2 = p2 - p0;
            //mat3的构造函数传入vec3是指定了列向量
            glm::vec3 solution = glm::inverse(glm::mat3(e1, e2, -dir)) * (pos - p0);
            if(solution.x > 0.f && solution.y > 0.f &&
               solution.x < 1.f && solution.y < 1.f &&
               solution.z > epsilon && (solution.x + solution.y <=1.f) &&
               (t_min < 0.f || solution.z < t_min)){
                t_min = solution.z;
                intersect = true;
                idx_mesh = i;
                idx_face = j;
            }
        }
        ++i;
    }
    if(!intersect)
        return glm::vec3(0, 0, 0);
    TriangleMesh& mesh = m_meshes[idx_mesh];
    glm::vec3 k_a = mesh.k_a, k_d = mesh.k_d, k_s = mesh.k_s;
//    if(std::abs(k_a.r - 0.2f) > 1e-5f){
//        qDebug() << "not wall";
//    }
    glm::vec3 pos_intersect = pos + dir * t_min;
    glm::vec3 ambient = k_a * m_light.color;
    glm::vec3 normal = m_meshes[idx_mesh].vertices[idx_face].normal;
    glm::vec3 light_dir = m_light.type == LIGHT_PARALLEL ? m_light.dir : (m_light.pos - pos_intersect);
    float diff = glm::dot(normal, light_dir);
    diff = diff > 0.f ? diff : 0.f;
    glm::vec3 disfuss = diff * m_light.color * k_d;
    glm::vec3 view_dir = glm::normalize(m_view_pos - pos_intersect);
    glm::vec3 reflect = glm::normalize(glm::reflect(-light_dir, normal));
    float spec = glm::pow(glm::max(glm::dot(view_dir, reflect), 0.f), 32.f);
    glm::vec3 specular = spec * m_light.color * k_s;

    glm::vec3 color_intersect = ambient + disfuss + specular;
    return color_intersect + k_s * computeLightRec(pos_intersect, reflect, k_s * weight, depth + 1);
//    return color_intersect;
}
