#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace tube {

struct ThreePoints;

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

	static ThreePoints divide(Point start, Point end, float t);
	static std::vector<glm::vec3> toVectors(Point start, Point end, int segments = 32);
	static std::vector<Point> toPoly(Point start, Point end, int segments = 32);
};

struct ThreePoints {
	Point A, B, C;
};

struct TwoPathes;
struct Shape;
class Tube;

struct Path {
	std::vector<Point> points;
	bool closed = false;

	TwoPathes divide(float t);
	Path slice(float start, float end);
	std::vector<Path> dash(float step);
	Path bevelJoin();
	Path roundJoin(float radius);
	Path miterJoin(float radius);
	Path withRoundedCaps(float radius);
	Path withSquareCaps(float radius);
	Path taper();
	Path copy();
	Path toPoly(int segmentsPerCurve = 32);
	Shape toShape(int segmentsPerCurve = 32);
};

struct TwoPathes {
	Path first;
	Path second;
};

struct Shape {
	std::vector<glm::vec3> verts;
	bool closed = false;
};

struct Builder {
	std::vector<Path> pathes;
	Shape shape;

	Builder(std::vector<Path> pathes, Shape shape);
	Builder(std::vector<Path> pathes);
	Builder(Path path);
	Builder(Shape shape);

	Builder withShape(Shape s);
	Builder withRoundedCaps(float radius);
	Builder withSquareCaps(float radius);
	Builder toPoly();
	Builder copy();
	Tube apply();
};

}