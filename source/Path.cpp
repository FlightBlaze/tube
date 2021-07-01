#include "Path.h"

using namespace tube;

float tube::lerpf(float a, float b, float t) {
    return a + t * (b - a);
}

std::vector<glm::vec3> tube::quadraticBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, int segments) {
    auto points = std::vector<glm::vec3>(segments);
    float step = 1.0f / (segments - 1);
    float t = 0.0f;
    for (int i = 0; i < segments; i++) {
        auto q0 = glm::mix(p0, p1, t);
        auto q1 = glm::mix(p1, p2, t);
        auto r  = glm::mix(q0, q1, t);
        points[i] = r;
        t += step;
    }
	return points;
}

std::vector<glm::vec3> tube::cubicBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int segments) {
	auto points = std::vector<glm::vec3>(segments);
    float step = 1.0f / (segments - 1);
    float t = 0.0f;
    for (int i = 0; i < segments; i++) {
        auto q0 = glm::mix(p0, p1, t);
        auto q1 = glm::mix(p1, p2, t);
        auto q2 = glm::mix(p2, p3, t);
        auto r0 = glm::mix(q0, q1, t);
        auto r1 = glm::mix(q1, q2, t);
        auto b  = glm::mix(r0, r1, t);
        points[i] = b;
        t += step;
    }
	return points;
}

Point::Point()
    : pos(glm::vec3(0.0f))
{
}

Point::Point(glm::vec3 pos)
    : pos(pos)
{
}

std::vector<Point> Point::toPoly(Point start, Point end, int segments) {
    std::vector<glm::vec3> verts;
    if (start.hasRightHandle && end.hasLeftHandle) {
        verts = cubicBezier(start.pos, start.rightHandlePos, end.leftHandlePos, end.pos, segments);
    }
    else if (start.hasRightHandle) {
        verts = quadraticBezier(start.pos, start.rightHandlePos, end.pos, segments);
    }
    else if (end.hasLeftHandle) {
        verts = quadraticBezier(start.pos, end.leftHandlePos, end.pos, segments);
    }
    else {
        verts = { start.pos, end.pos };
    }

    float length = 0.0f;
    std::vector<float> lengths(verts.size() - 1);
    for (int i = 0; i < verts.size() - 1; i++) {
        auto len = glm::length(verts[0] - verts[1]);
        lengths[i] = len;
        length += len;
    }

    std::vector<Point> points(verts.size());
    
    // Position of a vertex on a curve from 0 to 1
    float t = 0.0f;

    for (int i = 1; i < verts.size(); i++) {
        float radius = lerpf(start.radius, end.radius, t);
        float tilt = lerpf(start.tilt, end.tilt, t);

        auto point = Point(verts[i]);
        point.hasRightHandle = false;
        point.hasLeftHandle = false;
        point.radius = radius;
        point.tilt = tilt;
        points[i] = point;

        bool isEnd = i == verts.size() - 1;
        if(!isEnd)
            t += lengths[i] / length;
    }

	return points;
}

TwoPathes tube::Path::divide(float t)
{
	return TwoPathes();
}

Path tube::Path::toPoly(int segmentsPerCurve) {
    auto polypath = Path();
    polypath.closed = this->closed;

    for (size_t i = 0; i < this->points.size() - 1; i++) {
        bool isStart = i == 0;

        auto polycurve = Point::toPoly(this->points[i], this->points[i + 1LL], segmentsPerCurve);

        if (isStart)
            // Add all points of the curve to the path. There are no duplicates
            polypath.points.insert(polypath.points.begin() + polypath.points.size(),
                polycurve.begin(), polycurve.end());
        else
            // Add points without duplicates. The first point of this curve
            // is duplicate of the last point of the previous curve
            polypath.points.insert(polypath.points.begin() + polypath.points.size(),
                polycurve.begin() + 1, polycurve.end());
    }

    if (polypath.closed) {
        // Connect last point with first
        auto polycurve = Point::toPoly(this->points[0], this->points[this->points.size() - 1], segmentsPerCurve);

        // Add points without first and last because they are duplicates
        polypath.points.insert(polypath.points.begin() + polypath.points.size(),
            polycurve.begin() + 1, polycurve.end() - 1);
    }
    return polypath;
}
