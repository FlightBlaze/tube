#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <Path.h>

namespace tube {

enum class TubeCaps {
	TRIANGE_FAN,
	EAR_CUT
};

class Tube {
	void extrude(std::vector<glm::vec3> verts, bool shapeClosed);
	void bridge(int a1, int a2, int b1, int b2);
	void connectStartWithEnd(int shapeNumVertices);
	void triangleFan(int offset, int shapeVerts, int tipIndex);
	glm::vec3 getCentroidOfShape(int offset, int shapeVerts);

	int mShapeNumVerts = 0;
	bool mIsFirst = true;

	Tube();

public:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoords;
	std::vector<int> indices;

	Tube(Path path, Shape& shape);
	Tube(std::vector<Tube> tubes);

	Tube copy();

	// Fill caps in place and return this instance of class.
	// If you need to fill caps in copy only, consider using .copy() before .fillCaps()
	Tube fillCaps(TubeCaps capsType = TubeCaps::TRIANGE_FAN);

	// To positions and texture coordinates
	std::vector<float> toXYZUV();

	// To positions
	std::vector<float> toXYZ();

	static Tube join(Tube a, Tube& b);
};

class Shapes {
public:
	static Shape circle(float radius, int segments = 32);
};

}