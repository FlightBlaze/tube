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
	static float length(Point start, Point end);
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

	bool hasNonPoly();

	TwoPathes divide(float t);
	Path slice(float start, float end);
	std::vector<Path> dash(float step);
	Path bevelJoin(float radius);
	Path roundJoin(float radius);
	Path miterJoin(float radius);
	Path withRoundedCaps(float radius, int segments = 24);
	Path withSquareCaps(float radius);
	Path taper();
	Path copy();
	Path toPoly(int segmentsPerCurve = 32);
	Shape toShape(int segmentsPerCurve = 32);
	float length();

private:
	Path bevelOrRoundJoin(float radius, bool isRound);
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
	Builder bevelJoin(float radius);
	Builder roundJoin(float radius);
	Builder withRoundedCaps(float radius, int segments = 24);
	Builder withSquareCaps(float radius);
	Builder toPoly();
	Builder copy();
	Tube apply();
};

}