#include "geometry.h"
glm::vec3 aiColor3Toglm3(const aiColor3D& color){
    return glm::vec3(color.r, color.g, color.b);
}
