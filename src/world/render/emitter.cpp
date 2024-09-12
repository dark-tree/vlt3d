
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
	front = mesh.size();
	mesh.reserve(3);
}

void MeshEmitter::pushVertex(double x, double y, double z, float u, float v, int index, uint8_t r, uint8_t g, uint8_t b, Normal normal) {
	mesh.emplace_back(x, y, z, u, v, index, r, g, b, normal);
}

void MeshEmitter::pushIndex(size_t index) {
	mesh.push_back(mesh[back + index]);
}

void MeshEmitter::clear() {
	mesh.clear();
	front = 0;
	back = 0;
}

const std::vector<VertexTerrain>& MeshEmitter::getVertexData() const {
	return mesh;
}

void MeshEmitter::reserve(size_t elements) {
	mesh.reserve(elements);
}