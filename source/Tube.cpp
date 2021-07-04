#include "Tube.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace tube;

void Tube::extrude(std::vector<glm::vec3> verts, bool shapeClosed) {
	this->vertices.insert(this->vertices.end(), verts.begin(), verts.end());

	if (mIsFirst) {
		mIsFirst = false;
		return;
	}

	int firstPart  = (int)this->vertices.size() - (int)verts.size() * 2;
	int secondPart = (int)this->vertices.size() - (int)verts.size();

	for (int edge = 0; edge < (int)verts.size() - 1; edge++) {
		bridge(
			firstPart + edge,
			firstPart + edge + 1,
			secondPart + edge,
			secondPart + edge + 1
		);
	}

	if (shapeClosed) {
		int lastVertex = (int)verts.size() - 1;
		bridge(
			firstPart + lastVertex,
			firstPart,
			secondPart + lastVertex,
			secondPart
		);
	}
}

void Tube::bridge(int a1, int a2, int b1, int b2) {
	auto tris = std::vector<int>({
		b1, a1, a2, a2, b2, b1
	});
	this->indices.insert(this->indices.begin(), tris.begin(), tris.end());
}

void Tube::connectStartWithEnd(int shapeNumVertices) {
	int firstPart = 0;
	int secondPart = (int)this->vertices.size() - shapeNumVertices;

	for (int edge = 0; edge < shapeNumVertices - 1; edge++) {
		bridge(
			firstPart + edge,
			firstPart + edge + 1,
			secondPart + edge,
			secondPart + edge + 1
		);
	}
}

Tube::Tube(Path path, Shape& shape, TubeCaps caps) {
	if (path.hasNonPoly())
		path = path.toPoly();

	for (size_t i = 0; i < path.points.size(); i++) {
		bool isStart = i == 0;
		bool isEnd = i == path.points.size() - 1;

		Point curPoint = path.points[i];
		Point backPoint = !isStart ? path.points[i - 1LL] : curPoint;
		Point nextPoint = !isEnd   ? path.points[i + 1LL] : curPoint;

		glm::vec3 forwardDir  = glm::normalize(nextPoint.pos - curPoint.pos);
		glm::vec3 backwardDir = glm::normalize(backPoint.pos - curPoint.pos) * -1.0f;

		glm::vec3 meanDir;

		if      (isStart) meanDir = forwardDir;
		else if (isEnd)   meanDir = backwardDir;
		else              meanDir = glm::normalize((forwardDir + backwardDir) / 2.0f);

		glm::mat4 identity = glm::mat4(1.0f);
		glm::vec3 up = glm::vec3(0, 0, 1);
		glm::mat4 shapeMat = glm::translate(identity, curPoint.pos) *
			// glm::rotate(identity, glm::radians(90.0f), glm::vec3(1, 0, 0)) *
			// glm::lookAt(glm::vec3(0.0f), meanDir, up) *
			// glm::rotate(identity, curPoint.tilt, up) *
			glm::scale(identity, glm::vec3(curPoint.radius));

		glm::quat shapeTilt = glm::quat(glm::vec3(0, 0, curPoint.tilt));
		glm::quat shapeLookAt = glm::quatLookAt(meanDir, up);

		auto transformedShape = std::vector<glm::vec3>(shape.verts.size());
		for (int p = 0; p < shape.verts.size(); p++)
			transformedShape[p] = shapeMat * (shapeLookAt * shapeTilt * glm::vec4(shape.verts[p], 1.0f));

		extrude(transformedShape, shape.closed);

		if (path.closed)
			connectStartWithEnd((int)shape.verts.size());
	}
}

Tube::Tube(std::vector<Tube> tubes) {
	size_t indicesEnd = 0;
	for (auto tube : tubes) {
		this->vertices.insert(this->vertices.end(), tube.vertices.begin(), tube.vertices.end());
		this->indices.resize(this->indices.size() + tube.indices.size());
		for (size_t i = 0; i < tube.indices.size(); i++)
			this->indices[indicesEnd + i] = (int)indicesEnd + tube.indices[i];
		indicesEnd += tube.indices.size();
	}
}

std::vector<float> Tube::toXYZ() {
	auto attribs = std::vector<float>(this->vertices.size() * 3);
	for (size_t i = 0; i < vertices.size(); i++) {
		glm::vec3 vertex = vertices[i];
		attribs[i * 3LL      ] = vertex.x;
		attribs[i * 3LL + 1LL] = vertex.y;
		attribs[i * 3LL + 2LL] = vertex.z;
	}
	return attribs;
}

Tube Tube::join(Tube a, Tube& b) {
	size_t indicesEnd = a.indices.size();
	a.vertices.insert(a.vertices.end(), b.vertices.begin(), b.vertices.end());
	a.indices.resize(a.indices.size() + b.indices.size());
	for (size_t i = 0; i < b.indices.size(); i++)
		a.indices[indicesEnd + i] = (int)indicesEnd + b.indices[i];
	return a;
}

Shape Shapes::circle(float radius, int segments)
{
	auto shape = Shape();
	shape.closed = true;
	shape.verts = std::vector<glm::vec3>(segments);
	float angle = 0.0f;
	float arcLength = 360.0f;
	for (int i = 0; i < segments; i++) {
		float x = sin(glm::radians(angle)) * radius;
		float y = cos(glm::radians(angle)) * radius;

		shape.verts[i] = glm::vec3(x, y, 0.0f);

		angle += (arcLength / segments);
	}
	return shape;
}