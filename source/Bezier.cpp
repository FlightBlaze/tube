#include "Bezier.h"

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
        auto r = glm::mix(q0, q1, t);
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
        auto b = glm::mix(r0, r1, t);
        points[i] = b;
        t += step;
    }
    return points;
}

TwoQuadraticBeziers tube::divideQuadraticBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, float t) {
    auto q0 = glm::mix(p0, p1, t);
    auto q1 = glm::mix(p1, p2, t);
    auto r = glm::mix(q0, q1, t);
    TwoQuadraticBeziers two;
    two.a0 = p0;
    two.a1 = q0;
    two.a2 = r;
    two.b0 = r;
    two.b1 = q1;
    two.b2 = p2;
    return two;
}

TwoCubicBeziers tube::divideCubicBezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) {
    auto q0 = glm::mix(p0, p1, t);
    auto q1 = glm::mix(p1, p2, t);
    auto q2 = glm::mix(p2, p3, t);
    auto r0 = glm::mix(q0, q1, t);
    auto r1 = glm::mix(q1, q2, t);
    auto b = glm::mix(r0, r1, t);
    TwoCubicBeziers two;
    two.a0 = p0;
    two.a1 = q0;
    two.a2 = r0;
    two.a3 = b;
    two.b0 = b;
    two.b1 = r1;
    two.b2 = q2;
    two.b3 = p3;
    return two;
}