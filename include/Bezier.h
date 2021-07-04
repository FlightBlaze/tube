#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace tube {

// Linear interpolation
float lerpf(float a, float b, float t);

std::vector<glm::vec3> quadraticBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, int segments = 32);
std::vector<glm::vec3> cubicBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int segments = 32);

struct TwoQuadraticBeziers {
	// First curve
	glm::vec3 a0, a1, a2;
	// Second curve
	glm::vec3 b0, b1, b2;
};

TwoQuadraticBeziers divideQuadraticBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float t);

struct TwoCubicBeziers {
	// First curve
	glm::vec3 a0, a1, a2, a3;
	// Second curve
	glm::vec3 b0, b1, b2, b3;
};

TwoCubicBeziers divideCubicBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t);

}
