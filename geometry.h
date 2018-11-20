#ifndef GEOMETRY_H
#define GEOMETRY_H
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
    TriangleMesh(const vector<Vertex>& vertices, const vector<int>& indices):vertices(vertices), indices(indices){}
public:
    vector<Vertex> vertices;
    vector<int> indices;
};
#endif
