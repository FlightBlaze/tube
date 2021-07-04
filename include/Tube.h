#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <Path.h>

namespace tube {

enum class TubeCaps {
	NONE,
	FILL
};

class Tube {
	void extrude(std::vector<glm::vec3> verts, bool shapeClosed);
	void bridge(int a1, int a2, int b1, int b2);
	void connectStartWithEnd(int shapeNumVertices);
	bool mIsFirst = true;

public:
	std::vector<glm::vec3> vertices;
	std::vector<int> indices;

	Tube(Path path, Shape& shape, TubeCaps caps = TubeCaps::NONE);
	Tube(std::vector<Tube> tubes);

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