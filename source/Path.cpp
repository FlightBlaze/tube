#include "Path.h"
#include "Bezier.h"
#include "Tube.h"

using namespace tube;

Point::Point()
    : pos(glm::vec3(0.0f))
{
}

Point::Point(glm::vec3 pos)
    : pos(pos)
{
}

std::vector<glm::vec3> Point::toVectors(Point start, Point end, int segments) {
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
    return verts;
}

std::vector<Point> Point::toPoly(Point start, Point end, int segments) {
    auto verts = Point::toVectors(start, end, segments);

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
            polypath.points.insert(polypath.points.end(),
                polycurve.begin(), polycurve.end());
        else
            // Add points without duplicates. The first point of this curve
            // is duplicate of the last point of the previous curve
            polypath.points.insert(polypath.points.end(),
                polycurve.begin() + 1, polycurve.end());
    }

    if (polypath.closed) {
        // Connect last point with first
        auto polycurve = Point::toPoly(this->points[0], this->points[this->points.size() - 1], segmentsPerCurve);

        // Add points without first and last because they are duplicates
        polypath.points.insert(polypath.points.end(),
            polycurve.begin() + 1, polycurve.end() - 1);
    }
    return polypath;
}

// The same as toPoly
Shape tube::Path::toShape(int segmentsPerCurve) {
    auto shape = Shape();
    shape.closed = this->closed;

    for (size_t i = 0; i < this->points.size() - 1; i++) {
        bool isStart = i == 0;
        auto curve = Point::toVectors(this->points[i], this->points[i + 1LL], segmentsPerCurve);
        if (isStart)
            shape.verts.insert(shape.verts.end(),
                curve.begin(), curve.end());
        else
            shape.verts.insert(shape.verts.end(),
                curve.begin() + 1, curve.end());
    }

    if (shape.closed) {
        auto curve = Point::toVectors(this->points[0], this->points[this->points.size() - 1], segmentsPerCurve);
        shape.verts.insert(shape.verts.end(),
            curve.begin() + 1, curve.end() - 1);
    }
    return shape;
}

Builder::Builder(std::vector<Path> pathes, Shape shape)
    : pathes(pathes), shape(shape)
{
}


Builder::Builder(std::vector<Path> pathes)
    : pathes(pathes)
{
}

Builder::Builder(Path path)
    : pathes({ path })
{
}

Builder Builder::withShape(Shape s) {
    return Builder(this->pathes, s);
}

Tube tube::Builder::apply()
{
    std::vector<Tube> tubes;
    for (auto path : this->pathes)
        tubes.push_back(Tube(path, this->shape));
    return Tube(tubes);
}
