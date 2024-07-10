#pragma once

#include "external.hpp"

// for society's consideration, currently unused
struct Layout {
	enum XyzUvRgba { XYZ_UV_RGBA };
	enum XyUvRgba { XY_UV_RGBA };
};

struct Vertex3D { // TODO Rename to Vertex3D
	float x, y, z;
	float u, v;
	uint8_t r, g, b, a;

	Vertex3D() = default;

	[[deprecated("use explicit RGB bytes")]]
	Vertex3D(float x, float y, float z, float rf, float gf, float bf, float u, float v)
	: x(x), y(y), z(z), u(u), v(v), r(rf * 255), g(gf * 255), b(bf * 255), a(255) {}

	Vertex3D(float x, float y, float z, float u, float v, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	: x(x), y(y), z(z), u(u), v(v), r(r), g(g), b(b), a(a) {}
};


struct Vertex2D {
	float x, y;
	float u, v;
	uint8_t r, g, b, a;

	Vertex2D() = default;

	Vertex2D(float x, float y, float u, float v, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	: x(x), y(y), u(u), v(v), r(r), g(g), b(b), a(a) {}
};

// make sure our Vertices have the correct size
static_assert(sizeof(Vertex3D) <= sizeof(float) * 6);
static_assert(sizeof(Vertex2D) <= sizeof(float) * 5);