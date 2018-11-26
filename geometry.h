#ifndef GEOMETRY_H
#define GEOMETRY_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

using std::vector;
class Vertex {
public:
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 tex_uv;
};

//三角形网格，indices中相邻的3个表示一个面对应的顶点索引
class TriangleMesh {
public:
    TriangleMesh(const vector<Vertex>& vertices, const vector<int>& indices,
                 const glm::vec3& k_a, const glm::vec3& k_d, const glm::vec3& k_s,
                 const vector<int>& idx_textures):
        vertices(vertices), indices(indices),
        k_a(k_a), k_d(k_d), k_s(k_s),
        idx_textures(idx_textures){

    }
public:
    vector<Vertex> vertices;
    vector<int> indices;
    glm::vec3 k_a;
    glm::vec3 k_d;
    glm::vec3 k_s;
    vector<int> idx_textures;   //只保留索引而不保留具体存储什么数值，为信息交互提供一个统一的接口
};

struct FileTexture{
    aiTextureType type;
    int idx_image;
};      //GUI作为实现转换的中心，这个结构也应该放在公共头文件中

#define LIGHT_PARALLEL 0
#define LIGHT_POINT 1

struct LightParam{
    int type;
    union {
        glm::vec3 pos;
        glm::vec3 dir;
    };
    glm::vec3 color;
};

extern glm::vec3 aiColor3Toglm3(const aiColor3D& color);
#endif
