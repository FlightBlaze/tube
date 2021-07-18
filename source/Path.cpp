#include "..\include\Path.h"
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

Point::Point(glm::vec3 pos, glm::vec3 rightHandle)
    : pos(pos), rightHandlePos(rightHandle), hasRightHandle(true)
{
}

Point::Point(glm::vec3 pos, glm::vec3 leftHandle, glm::vec3 rightHandle)
    : pos(pos), leftHandlePos(leftHandle), rightHandlePos(rightHandle),
        hasLeftHandle(true), hasRightHandle(true)
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

float tube::Point::length(Point start, Point end) {
    auto poly = Point::toPoly(start, end);
    glm::vec3 previousPos = poly[0].pos;
    float len = 0;
    for (int i = 1; i < poly.size(); i++) {
        len += glm::length(poly[i].pos - previousPos);
        previousPos = poly[i].pos;
    }
    return len;
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

bool tube::Path::hasNonPoly() {
    for (const auto& point : points) {
        if (point.hasLeftHandle || point.hasRightHandle)
            return true;
    }
    return false;
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

Path tube::Path::close() {
    assert(this->points.size() > 0);
    auto path = this->copy();
    auto end = tube::Point();
    end.pos = path.points[0].pos;
    end.radius = path.points[0].radius;
    end.tilt = path.points[0].tilt;
    end.leftHandlePos = path.points[0].leftHandlePos;
    end.hasLeftHandle = path.points[0].hasLeftHandle;
    path.points.push_back(end);
    return path;
}

// #include <iostream>

float bevel_t(float len, float r = 0.05f) {
    return (len - r) / len;
}

Path tube::Path::bevelOrRoundJoin(float radius, bool isRound) {
    Path path = this->copy();
    const float r = radius / 2;

    for (size_t i = 0; i + 2 < path.points.size(); i += 2) {
        float tA =        bevel_t(Point::length(path.points[i + 0], path.points[i + 1]), r);
        float tB = 1.0f - bevel_t(Point::length(path.points[i + 1], path.points[i + 2]), r);

        auto upperArm = Point::divide(path.points[i + 0], path.points[i + 1], tA);
        if (isRound) {
            upperArm.B.hasRightHandle = true;
            upperArm.B.rightHandlePos = upperArm.C.pos;
        }
        else
            upperArm.B.hasRightHandle = false;

        auto lowerArm = Point::divide(path.points[i + 1], path.points[i + 2], tB);
        lowerArm.B.hasLeftHandle = false;

        auto newPoints = std::vector<Point>({
            upperArm.A, upperArm.B,
            lowerArm.B, lowerArm.C
            });

        path.points.erase(path.points.begin() + i, path.points.begin() + i + 3);
        path.points.insert(path.points.begin() + i, newPoints.begin(), newPoints.end());
    }
    return path;
}

Path tube::Path::bevelJoin(float radius) {
    return bevelOrRoundJoin(radius, false);
}

Path tube::Path::roundJoin(float radius) {
    return bevelOrRoundJoin(radius, true);
}

float straightToRound(float t) {
    return sqrt(1 - t * t);
}

Path tube::Path::withRoundedCaps(float radius, int segments) {
    // First curve divided at the beginning
    auto startDivided = Point::divide(this->points[0], this->points[1], 0.1f);

    glm::vec3 startDir = glm::normalize(startDivided.A.pos - startDivided.B.pos);
    float startRadius = this->points[0].radius;
    float startTilt = this->points[0].tilt;
    std::vector<Point> start(segments);
    for (int i = 0; i < start.size(); i++) {
        float t = ((float)segments - (float)i) / ((float)segments + 1.0f);
        float r = 1.0f - t;
        start[i].pos = this->points[0].pos + startDir * radius * straightToRound(r);
        start[i].radius = startRadius * r;
        start[i].tilt = startTilt;
        // std::cout << start[i].radius << std::endl;
    }

    // Last curve divided at the end
    auto endDivided = Point::divide(
        this->points[this->points.size() - 1],
        this->points[this->points.size() - 2],
        0.9f);

    glm::vec3 endDir = glm::normalize(endDivided.A.pos - endDivided.B.pos);
    float endRadius = this->points.back().radius;
    float endTilt = this->points.back().tilt;
    std::vector<Point> end(segments);
    for (int i = 0; i < end.size(); i++) {
        float t = ((float)segments - (float)i) / ((float)segments + 1.0f);
        end[i].pos = this->points.back().pos + endDir * radius * straightToRound(t);
        end[i].radius = endRadius * t;
        end[i].tilt = endTilt;
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
    float startTilt = this->points[0].tilt;
    Point start;
    start.pos = this->points[0].pos + startDir * radius;
    start.radius = startRadius;
    start.tilt = startTilt;

    // Last curve divided at the end
    auto endDivided = Point::divide(
        this->points[this->points.size() - 1],
        this->points[this->points.size() - 2],
        0.9f);

    glm::vec3 endDir = glm::normalize(endDivided.A.pos - endDivided.B.pos);
    float endRadius = this->points.back().radius;
    float endTilt = this->points.back().tilt;
    Point end;
    end.pos = this->points.back().pos + endDir * radius;
    end.radius = endRadius;
    end.tilt = endTilt;

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
        auto polycurve = Point::toPoly(this->points[this->points.size() - 1], this->points[0], segmentsPerCurve);

        // Add points without first because they are duplicate
        // polypath.points.clear();
        polypath.points.insert(polypath.points.end(),
            polycurve.begin() + 1, polycurve.end());
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
        auto curve = Point::toVectors(this->points[this->points.size() - 1], this->points[0], segmentsPerCurve);
        shape.verts.insert(shape.verts.end(),
            curve.begin() + 1, curve.end());
    }
    return shape;
}

float tube::Path::length() {
    if (this->points.size() < 2)
        return 0;

    glm::vec3 previousPos = this->points[0].pos;
    float len = 0;
    if(this->hasNonPoly()) {
        auto path = this->toPoly();
        for (int i = 1; i < path.points.size(); i++) {
            len += glm::length(path.points[i].pos - previousPos);
            previousPos = path.points[i].pos;
        }
    }
    else {
        for (int i = 1; i < this->points.size(); i++) {
            len += glm::length(this->points[i].pos - previousPos);
            previousPos = this->points[i].pos;
        }
    }
    return len;
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

Builder tube::Builder::bevelJoin(float radius) {
    auto builder = Builder(this->shape);
    builder.pathes.resize(this->pathes.size());
    for (int i = 0; i < this->pathes.size(); i++)
        builder.pathes[i] = this->pathes[i].bevelJoin(radius);
    return builder;
}

Builder tube::Builder::roundJoin(float radius) {
    auto builder = Builder(this->shape);
    builder.pathes.resize(this->pathes.size());
    for (int i = 0; i < this->pathes.size(); i++)
        builder.pathes[i] = this->pathes[i].roundJoin(radius);
    return builder;
}


Builder tube::Builder::withRoundedCaps(float radius, int segments) {
    auto builder = Builder(this->shape);
    builder.pathes.resize(this->pathes.size());
    for (int i = 0; i < this->pathes.size(); i++)
        builder.pathes[i] = this->pathes[i].withRoundedCaps(radius, segments);
    return builder;
}

Builder tube::Builder::withSquareCaps(float radius) {
    auto builder = Builder(this->shape);
    builder.pathes.resize(this->pathes.size());
    for (int i = 0; i < this->pathes.size(); i++)
        builder.pathes[i] = this->pathes[i].withSquareCaps(radius);
    return builder;
}

Builder tube::Builder::toPoly() {
    auto builder = Builder(this->shape);
    builder.pathes.resize(this->pathes.size());
    for (int i = 0; i < this->pathes.size(); i++)
        builder.pathes[i] = this->pathes[i].toPoly();
    return builder;
}

Builder tube::Builder::copy() {
    return Builder(this->pathes, this->shape);
}

Tube tube::Builder::apply() {
    std::vector<Tube> tubes;
    for (auto path : this->pathes)
        tubes.push_back(Tube(path, this->shape));
    return Tube(tubes);
}
