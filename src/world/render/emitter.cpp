
#include "emitter.hpp"

/*
 * MeshEmitter
 */

MeshEmitter::MeshEmitter() {
	front = 0;
	back = 0;
}

void MeshEmitter::nextTriangle() {
	back = front;
	front = vertices.size();
	vertices.reserve(3);
}

void MeshEmitter::pushVertex(double x, double y, double z, float u, float v, int index, uint8_t r, uint8_t g, uint8_t b, Normal normal) {
	vertices.emplace_back(x, y, z, u, v, index, r, g, b, normal);
}

void MeshEmitter::pushIndex(size_t index) {
	vertices.push_back(vertices[back + index]);
}

void MeshEmitter::clear() {
	vertices.clear();
	front = 0;
	back = 0;
}

const std::vector<VertexTerrain>& MeshEmitter::getVertexData() const {
	return vertices;
}

void MeshEmitter::reserve(size_t elements) {
	vertices.reserve(elements);
}

size_t MeshEmitter::size() const {
	return vertices.size();
}