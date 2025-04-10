#pragma once

#include "external.hpp"

enum struct Normal : uint8_t {
	WEST       = 0, // {-1, 0, 0}
	EAST       = 1, // {+1, 0, 0}
	DOWN       = 2, // {0, -1, 0}
	UP         = 3, // {0, +1, 0}
	NORTH      = 4, // {0, 0, -1}
	SOUTH      = 5, // {0, 0, +1}
};

/**
 * Used by the chunk bounding box to perform occlusion testing
 */
struct VertexOcclusion {
	float x, y, z;
};

/**
 * Used by the terrain chunk data, relies on texture arrays
 */
struct VertexTerrain {
	float x, y, z;
	uint32_t i;
	uint32_t uv;
	uint8_t r, g, b;
	Normal normal;

	VertexTerrain() = default;
	VertexTerrain(float x, float y, float z, float u, float v, int index, uint8_t r, uint8_t g, uint8_t b, Normal normal)
	: x(x), y(y), z(z), i(index), uv(glm::packHalf2x16({u, v})), r(r), g(g), b(b), normal(normal) {}
};

/**
 * Generic 3D vertex, used by the ImmediateRenderer
 */
struct Vertex3D {
	float x, y, z;
	float u, v;
	uint8_t r, g, b, a;

	Vertex3D() = default;
	Vertex3D(float x, float y, float z, float u, float v, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	: x(x), y(y), z(z), u(u), v(v), r(r), g(g), b(b), a(a) {}
};

/**
 * Generic 2D vertex, used by the ImmediateRenderer
 */
struct Vertex2D {
	float x, y;
	float u, v;
	uint8_t r, g, b, a;

	Vertex2D() = default;
	Vertex2D(float x, float y, float u, float v, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	: x(x), y(y), u(u), v(v), r(r), g(g), b(b), a(a) {}
};

// make sure our Vertices have the correct size
static_assert(sizeof(VertexOcclusion) == 12);
static_assert(sizeof(VertexTerrain) == 24);
static_assert(sizeof(Vertex3D) == 24);
static_assert(sizeof(Vertex2D) == 20);
