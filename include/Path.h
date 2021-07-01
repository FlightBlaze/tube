#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace tube {

float lerpf(float a, float b, float t);

std::vector<glm::vec3> quadraticBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, int segments = 32);
std::vector<glm::vec3> cubicBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int segments = 32);

struct Point {
	glm::vec3 pos;
	glm::vec3 rightHandlePos = glm::vec3(0.0f);
	glm::vec3 leftHandlePos = glm::vec3(0.0f);
	bool hasRightHandle = false;
	bool hasLeftHandle = false;
	float radius = 1.0f;
	float tilt = 0.0f;

	Point();
	Point(glm::vec3 pos);

	static std::vector<Point> divide(Point start, Point end, float t);
	static std::vector<Point> toPoly(Point start, Point end, int segments = 32);
};

struct TwoPathes;

struct Path {
	std::vector<Point> points;
	bool closed = false;

	TwoPathes divide(float t);
	Path slice(float start, float end);
	std::vector<Path> dash(float step);
	Path bevelJoin();
	Path withRoundedCaps();
	Path toPoly(int segmentsPerCurve = 32);
};

struct TwoPathes {
	Path first;
	Path second;
};

class Pathes {
public:
	std::vector<Path> pathes;
};

}