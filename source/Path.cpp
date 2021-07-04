#include "..\include\Path.h"
#include "..\include\Path.h"
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

ThreePoints tube::Point::divide(Point start, Point end, float t)
{
    ThreePoints points;
    if (start.hasRightHandle && end.hasLeftHandle) {
        auto twoCurves = divideCubicBezier(start.pos, start.rightHandlePos, end.leftHandlePos, end.pos, t);

        Point a = Point(twoCurves.a0);
        a.hasLeftHandle = start.hasLeftHandle;
        a.leftHandlePos = start.leftHandlePos;
        a.hasRightHandle = true;
        a.rightHandlePos = twoCurves.a1;
        a.radius = start.radius;
        a.tilt = start.tilt;

        Point b = Point(twoCurves.b0);
        b.hasLeftHandle = true;
        b.leftHandlePos = twoCurves.a2;
        b.hasRightHandle = true;
        b.rightHandlePos = twoCurves.b1;
        b.radius = lerpf(start.radius, end.radius, t);
        b.tilt = lerpf(start.tilt, end.tilt, t);

        Point c = Point(twoCurves.b3);
        c.hasRightHandle = end.hasRightHandle;
        c.rightHandlePos = end.rightHandlePos;
        c.hasLeftHandle = true;
        c.leftHandlePos = twoCurves.b2;
        c.radius = end.radius;
        c.tilt = end.tilt;

        points = { a, b, c };
    }
    else if (start.hasRightHandle || end.hasLeftHandle) {
        TwoQuadraticBeziers twoCurves;
        if(start.hasRightHandle)
            twoCurves = divideQuadraticBezier(start.pos, start.rightHandlePos, end.pos, t);
        if (end.hasLeftHandle)
            twoCurves = divideQuadraticBezier(start.pos, end.leftHandlePos, end.pos, t);

        Point a = Point(twoCurves.a0);
        a.hasLeftHandle = start.hasLeftHandle;
        a.leftHandlePos = start.leftHandlePos;
        a.hasRightHandle = false;
        a.radius = start.radius;
        a.tilt = start.tilt;

        Point b = Point(twoCurves.b0);
        b.hasLeftHandle = true;
        b.leftHandlePos = twoCurves.a1;
        b.hasRightHandle = true;
        b.rightHandlePos = twoCurves.b1;
        b.radius = lerpf(start.radius, end.radius, t);
        b.tilt = lerpf(start.tilt, end.tilt, t);

        Point c = Point(twoCurves.b2);
        c.hasRightHandle = end.hasRightHandle;
        c.rightHandlePos = end.rightHandlePos;
        c.hasLeftHandle = false;
        c.radius = end.radius;
        c.tilt = end.tilt;

        points = { a, b, c };
    }
    else {
        Point mid = Point(glm::mix(start.pos, end.pos, t));
        mid.radius = lerpf(start.radius, end.radius, t);
        mid.tilt = lerpf(start.tilt, end.tilt, t);
        points = { start, mid, end };
    }
    return points;
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

    for (int i = 0; i < verts.size(); i++) {
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

Path tube::Path::copy() {
    Path path;
    path.closed = this->closed;
    path.points = this->points;
    return path;
}

// #include <iostream>

float straightToRound(float t) {
    return sqrt(1 - t * t);
}

Path tube::Path::withRoundedCaps(float radius) {
    int segments = 16;

    // First curve divided at the beginning
    auto startDivided = Point::divide(this->points[0], this->points[1], 0.1f);

    glm::vec3 startDir = glm::normalize(startDivided.A.pos - startDivided.B.pos);
    float startRadius = this->points[0].radius;
    std::vector<Point> start(segments);
    for (int i = 0; i < start.size(); i++) {
        float r = straightToRound(((float)segments - (float)i - 1.0f) / (float)segments);
        start[i].pos = this->points[0].pos - startDir * (radius / (float)segments * (float)i) + startDir * radius;
        start[i].radius = startRadius * r;
        // std::cout << start[i].radius << std::endl;
    }

    // Last curve divided at the end
    auto endDivided = Point::divide(
        this->points[this->points.size() - 1],
        this->points[this->points.size() - 2],
        0.9f);

    glm::vec3 endDir = glm::normalize(endDivided.A.pos - endDivided.B.pos);
    float endRadius = this->points.back().radius;
    std::vector<Point> end(segments);
    for (int i = 0; i < end.size(); i++) {
        float r = straightToRound((float)i / (float)segments);
        end[i].pos = this->points.back().pos + endDir * (radius / (float)segments * ((float)i + 1.0f));
        end[i].radius = endRadius * r;
    }

    Path path = this->copy();
    path.points[0].hasLeftHandle = false;
    path.points.back().hasRightHandle = false;
    path.points.insert(path.points.begin(), start.begin(), start.end());
    path.points.insert(path.points.end(), end.begin(), end.end());
    return path;
}

Path tube::Path::withSquareCaps(float radius) {
    // First curve divided at the beginning
    auto startDivided = Point::divide(this->points[0], this->points[1], 0.1f);

    glm::vec3 startDir = glm::normalize(startDivided.A.pos - startDivided.B.pos);
    float startRadius = this->points[0].radius;
    Point start;
    start.pos = this->points[0].pos + startDir * radius;
    start.radius = startRadius;

    // Last curve divided at the end
    auto endDivided = Point::divide(
        this->points[this->points.size() - 1],
        this->points[this->points.size() - 2],
        0.9f);

    glm::vec3 endDir = glm::normalize(endDivided.A.pos - endDivided.B.pos);
    float endRadius = this->points.back().radius;
    Point end;
    end.pos = this->points.back().pos + endDir * radius;
    end.radius = endRadius;

    Path path = this->copy();
    path.points[0].hasLeftHandle = false;
    path.points.back().hasRightHandle = false;
    path.points.insert(path.points.begin(), start);
    path.points.insert(path.points.end(), end);
    return path;
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

Builder::Builder(Shape shape)
    : shape(shape)
{
}

Builder Builder::withShape(Shape s) {
    return Builder(this->pathes, s);
}

Builder tube::Builder::withRoundedCaps(float radius)
{
    auto builder = Builder(this->shape);
    builder.pathes.resize(this->pathes.size());
    for (int i = 0; i < this->pathes.size(); i++)
        builder.pathes[i] = this->pathes[i].withRoundedCaps(radius);
    return builder;
}

Builder tube::Builder::withSquareCaps(float radius)
{
    auto builder = Builder(this->shape);
    builder.pathes.resize(this->pathes.size());
    for (int i = 0; i < this->pathes.size(); i++)
        builder.pathes[i] = this->pathes[i].withSquareCaps(radius);
    return builder;
}

Builder tube::Builder::toPoly()
{
    auto builder = Builder(this->shape);
    builder.pathes.resize(this->pathes.size());
    for (int i = 0; i < this->pathes.size(); i++)
        builder.pathes[i] = this->pathes[i].toPoly();
    return builder;
}

Builder tube::Builder::copy() {
    return Builder(this->pathes, this->shape);
}

Tube tube::Builder::apply()
{
    std::vector<Tube> tubes;
    for (auto path : this->pathes)
        tubes.push_back(Tube(path, this->shape));
    return Tube(tubes);
}
