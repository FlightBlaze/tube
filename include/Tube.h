#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <Path.h>

namespace tube {

enum class TubeCaps {
	NONE,
	FILL,
	ROUNDED,
	SQUARE,
	CLOSED
};

enum class TubeJoin {
	CONNECT,
	BEVEL,
	ROUND,
	MITER
};

class Tube {
	void extrude(std::vector<glm::vec3> verts, bool shapeClosed);
	void bridge(int a1, int a2, int b1, int b2);
	void connectStartWithEnd(int shapeNumVertices);
	bool mIsFirst = true;

public:
	std::vector<glm::vec3> vertices;
	std::vector<int> indices;

	Tube(Path& path, std::vector<glm::vec3> shape, bool shapeClosed = true, TubeCaps caps = TubeCaps::NONE, TubeJoin = TubeJoin::CONNECT);

	std::vector<float> toXYZ();
};

class TubeShapes {
public:
	static std::vector<glm::vec3> circle(float radius, int segments = 32);
};

}