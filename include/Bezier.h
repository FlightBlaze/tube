#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace tube {

// Linear interpolation
float lerpf(float a, float b, float t);

std::vector<glm::vec3> quadraticBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, int segments = 32);
std::vector<glm::vec3> cubicBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int segments = 32);

}
