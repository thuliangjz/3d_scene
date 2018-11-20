#ifndef GEOMETRY_H
#define GEOMETRY_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/vec3.hpp>
#include <vector>

using std::vector;
class Vertex {
public:
    glm::vec3 pos;
    glm::vec3 normal;
};

//三角形网格，indices中相邻的3个表示一个面对应的顶点索引
class TriangleMesh {
public:
    TriangleMesh(const vector<Vertex>& vertices, const vector<int>& indices, const glm::vec3& k_a, const glm::vec3& k_d, const glm::vec3& k_s):
        vertices(vertices), indices(indices),
        k_a(k_a), k_d(k_d), k_s(k_s){

    }
public:
    vector<Vertex> vertices;
    vector<int> indices;
    glm::vec3 k_a;
    glm::vec3 k_d;
    glm::vec3 k_s;
};

extern glm::vec3 aiColor3Toglm3(const aiColor3D& color);
#endif
