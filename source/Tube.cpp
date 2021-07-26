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
	/*
	if (shapeClosed) {
		int lastVertex = (int)verts.size() - 1;
		bridge(
			firstPart + lastVertex,
			firstPart,
			secondPart + lastVertex,
			secondPart
		);
	}
	*/
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

void Tube::triangleFan(int offset, int shapeVerts, int tipIndex) {
	size_t amount = (size_t)shapeVerts * 3;

	auto tris = std::vector<int>(amount);

	int k = 0;
	for (size_t i = 0; i < amount - 3; i += 3) {
		size_t first = 1;
		size_t second = 2;

		tris[i] = tipIndex;

		if (k == shapeVerts - 2) {
			// fill gap between first and last edge
			tris[i + first] = k + offset;
			tris[i + second] = offset;
		}
		else
		{
			// connect current edge with next
			tris[i + first] = k + offset;
			tris[i + second] = k + 1 + offset;
			k++;
		}
	}
	this->indices.insert(this->indices.end(), tris.begin(), tris.end());
}

glm::vec3 Tube::getCentroidOfShape(int offset, int shapeVerts) {
	auto sum = glm::vec3(0.0f);
	for (int i = offset; i < offset + shapeVerts; i++)
		sum += this->vertices[i];
	return sum / (float)shapeVerts;
}

// #include <iostream>

Tube::Tube(Path path, Shape& shape) {
	if (path.hasNonPoly())
		path = path.toPoly();
	else if(path.closed)
		path = path.close();

	// Need for generating texture coordinates

	float pathLength = path.length();
	float curLength = 0.0f;

	for (size_t i = 0; i < path.points.size(); i++) {
		bool isStart = i == 0;
		bool isEnd = i == path.points.size() - 1;

		Point curPoint = path.points[i];
		Point backPoint = !isStart ? path.points[i - 1LL] : curPoint;
		Point nextPoint = !isEnd   ? path.points[i + 1LL] : curPoint;
		
		if (isStart && path.closed)
			backPoint = path.points[path.points.size() - 2];

		else if (isEnd && path.closed)
			nextPoint = path.points[1];
		
		glm::vec3 forwardDir  = glm::normalize(nextPoint.pos - curPoint.pos);
		glm::vec3 backwardDir = glm::normalize(backPoint.pos - curPoint.pos) * -1.0f;

		glm::vec3 meanDir;

		if      (isStart && !path.closed) meanDir = forwardDir;
		else if (isEnd && !path.closed)   meanDir = backwardDir;
		else                              meanDir = glm::normalize((forwardDir + backwardDir) / 2.0f);

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

		// Generate texture coordinates

		float shapeEnd = (float)shape.verts.size() - 1.0f;
		for (int p = 0; p < shape.verts.size(); p++) {
			float u = p / shapeEnd;
			float v = curLength / pathLength;
			// std::cout << u << ", " << v << std::endl;
			texCoords.push_back(glm::vec2(u, v));
		}
		curLength += glm::length(curPoint.pos - nextPoint.pos);
	}
	// if (path.closed)
	//	connectStartWithEnd((int)shape.verts.size());

	this->mShapeNumVerts = (int)shape.verts.size();
}

Tube::Tube(std::vector<Tube> tubes) {
	if (tubes.size() == 0)
		return;
	this->mShapeNumVerts = tubes[0].mShapeNumVerts;
	size_t indicesEnd = 0;
	for (auto tube : tubes) {
		this->vertices.insert(this->vertices.end(), tube.vertices.begin(), tube.vertices.end());
		this->normals.insert(this->normals.end(), tube.normals.begin(), tube.normals.end());
		this->texCoords.insert(this->texCoords.end(), tube.texCoords.begin(), tube.texCoords.end());
		this->indices.resize(this->indices.size() + tube.indices.size());
		for (size_t i = 0; i < tube.indices.size(); i++)
			this->indices[indicesEnd + i] = (int)indicesEnd + tube.indices[i];
		indicesEnd += tube.indices.size();
	}
}

Tube::Tube()
{
}

Tube Tube::copy() {
	auto c = Tube();
	c.mShapeNumVerts = this->mShapeNumVerts;
	c.vertices.insert(c.vertices.end(), this->vertices.begin(), this->vertices.end());
	c.normals.insert(c.normals.end(), this->normals.begin(), this->normals.end());
	c.texCoords.insert(c.texCoords.end(), this->texCoords.begin(), this->texCoords.end());
	c.indices.resize(c.indices.size() + this->indices.size());
	for (size_t i = 0; i < this->indices.size(); i++)
		c.indices[i] = this->indices[i];
	return c;
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

std::vector<float> Tube::toXYZUV() {
	assert(this->vertices.size() == this->texCoords.size());

	auto attribs = std::vector<float>(this->vertices.size() * 5);
	for (size_t i = 0; i < vertices.size(); i++) {
		glm::vec3 vertex = vertices[i];
		glm::vec2 texCoord = texCoords[i];
		// std::cout << texCoord.x << ", " << texCoord.y << std::endl;
		attribs[i * 5LL] = vertex.x;
		attribs[i * 5LL + 1LL] = vertex.y;
		attribs[i * 5LL + 2LL] = vertex.z;
		attribs[i * 5LL + 3LL] = texCoord.x;
		attribs[i * 5LL + 4LL] = texCoord.y;
	}
	return attribs;
}

std::vector<float> Tube::toXYZUVNormal() {
	assert(this->vertices.size() == this->texCoords.size());
	assert(this->vertices.size() == this->normals.size());

	auto attribs = std::vector<float>(this->vertices.size() * 8);
	for (size_t i = 0; i < vertices.size(); i++) {
		glm::vec3 vertex = vertices[i];
		glm::vec2 texCoord = texCoords[i];
		glm::vec3 normal = normals[i];
		attribs[i * 8LL] = vertex.x;
		attribs[i * 8LL + 1LL] = vertex.y;
		attribs[i * 8LL + 2LL] = vertex.z;
		attribs[i * 8LL + 3LL] = texCoord.x;
		attribs[i * 8LL + 4LL] = texCoord.y;
		attribs[i * 8LL + 5LL] = normal.x;
		attribs[i * 8LL + 6LL] = normal.y;
		attribs[i * 8LL + 7LL] = normal.z;
	}
	return attribs;
}

Tube Tube::fillCaps(TubeCaps capsType) {
	if (capsType == TubeCaps::TRIANGE_FAN) {
		int start = 0;
		int end = (int)this->vertices.size() - mShapeNumVerts;

		glm::vec3 startCenter = getCentroidOfShape(start, mShapeNumVerts);
		glm::vec3 endCenter = getCentroidOfShape(end, mShapeNumVerts);

		auto tips = std::vector<glm::vec3>({ startCenter, endCenter });
		this->vertices.insert(this->vertices.end(), tips.begin(), tips.end());

		auto tipTexCoords = std::vector<glm::vec2>({ glm::vec2(0.5f, 0.0f), glm::vec2(0.5f, 1.0f) });
		this->texCoords.insert(this->texCoords.end(), tipTexCoords.begin(), tipTexCoords.end());

		int startTipIdx = (int)this->vertices.size() - 2;
		int endTipIdx = (int)this->vertices.size() - 1;

		// Fill the first loop with triangles
		triangleFan(start, mShapeNumVerts, startTipIdx);

		// Fill the last loop with triangles without last vertex
		// (start tip) to not connect start tip with end tip
		triangleFan(end, mShapeNumVerts - 1, endTipIdx);
	}
	return *this;
}

glm::vec3 computeFaceNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
	// Uses p2 as a new origin for p1,p3
	auto a = p3 - p2;
	auto b = p1 - p2;
	// Compute the cross product a X b to get the face normal
	return glm::normalize(glm::cross(a, b));
}

Tube Tube::calculateNormals() {
	this->normals = std::vector<glm::vec3>(this->vertices.size());
	// For each face calculate normals and append to the corresponding vertices of the face
	for (unsigned int i = 0; i < this->indices.size(); i += 3) {
		//vi v(i+1) v(i+2) are the three faces of a triangle
		glm::vec3 A = this->vertices[this->indices[i]];
		glm::vec3 B = this->vertices[this->indices[i + 1LL]];
		glm::vec3 C = this->vertices[this->indices[i + 2LL]];
		glm::vec3 normal = computeFaceNormal(A, B, C);
		this->normals[this->indices[i]] += normal;
		this->normals[this->indices[i + 1LL]] += normal;
		this->normals[this->indices[i + 2LL]] += normal;
	}
	// Normalize each normal
	for (unsigned int i = 0; i < this->normals.size(); i++)
		this->normals[i] = glm::normalize(this->normals[i]);

	return *this;
}

Tube Tube::join(Tube a, Tube& b) {
	size_t indicesEnd = a.indices.size();
	a.mShapeNumVerts = b.mShapeNumVerts;
	a.vertices.insert(a.vertices.end(), b.vertices.begin(), b.vertices.end());
	a.normals.insert(a.normals.end(), b.normals.begin(), b.normals.end());
	a.texCoords.insert(a.texCoords.end(), b.texCoords.begin(), b.texCoords.end());
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

		angle += (arcLength / (segments - 1));
	}
	return shape;
}