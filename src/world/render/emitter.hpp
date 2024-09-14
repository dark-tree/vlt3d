#pragma once

#include "external.hpp"
#include "client/vertices.hpp"
#include "buffer/buffer.hpp"

class MeshEmitter {

	private:

		size_t front, back;
		std::vector<VertexTerrain> mesh;

	public:

		MeshEmitter();

		/// Begin next triangle, must be called for indexing to work
		void nextTriangle();

		/// Emits a TerrainVertex
		void pushVertex(double x, double y, double z, float u, float v, int index, uint8_t r, uint8_t g, uint8_t b, Normal normal);

		/// Emits a TerrainVertex from the previous triangle by its index
		void pushIndex(size_t index);

	public:

		/// Clears the internal buffers
		void clear();

		/// Returns a read-only reference to the vertex data
		const std::vector<VertexTerrain>& getVertexData() const;

		/// Resizes the buffer to fit at least `count` elements
		void reserve(size_t count);

		/// Get mesh size (vertex count)
		size_t size() const;

};

class MeshEmitterSet {

	private:

		static constexpr size_t components = 7;
		mutable std::array<MeshEmitter, components> emitters;

	public:

		MeshEmitterSet(size_t size) {
			std::ranges::for_each(emitters, [size] (auto& emitter) {
				emitter.reserve(size);
			});
		}

		inline void clear() {
			std::ranges::for_each(emitters, [] (auto& emitter) {
				return emitter.clear();
			});
		}

		inline bool empty() const {
			return std::ranges::all_of(emitters, [] (const auto& emitter) {
				return emitter.getVertexData().empty();
			});
		}

		inline size_t bytes() const {
			return std::transform_reduce(emitters.cbegin(), emitters.cend(), 0, std::plus<size_t> {}, [] (const auto& mesh) {
				return mesh.getVertexData().size();
			}) * sizeof(VertexTerrain);
		}

	public:

		MeshEmitter& getWest() {
			return emitters[0];
		}

		MeshEmitter& getEast() {
			return emitters[1];
		}

		MeshEmitter& getDown() {
			return emitters[2];
		}

		MeshEmitter& getUp() {
			return emitters[3];
		}

		MeshEmitter& getNorth() {
			return emitters[4];
		}

		MeshEmitter& getSouth() {
			return emitters[5];
		}

		MeshEmitter& getGeneric() {
			return emitters[6];
		}

	public:

		void writeToBuffer(BasicBuffer& buffer, std::array<uint32_t, components>& region_begin, std::array<uint32_t, components>& region_count) const {

			MemoryMap::View memory = buffer.getMemoryView();
			size_t vertices = 0;

			for (int i = 0; i < components; i ++) {
				const auto& mesh = emitters[i].getVertexData();
				const size_t count = mesh.size();

				memory.write(mesh.data(), count * sizeof(VertexTerrain));

				region_begin[i] = vertices;
				region_count[i] = count;
				vertices += count;
			}

			buffer.count = vertices;
			buffer.bytes = vertices * sizeof(VertexTerrain);

		}

};
