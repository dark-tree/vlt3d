#pragma once

#include "external.hpp"
#include "world.hpp"
#include "client/vertices.hpp"
#include "util/logger.hpp"
#include "util/threads.hpp"
#include "client/renderer.hpp"

class WorldRenderer;

class WorldRenderView {

	private:

		int indexOf(int x, int y, int z) {
			int dx = (origin.x - x) + 1;
			int dy = (origin.y - y) + 1;
			int dz = (origin.z - z) + 1;

			return dx + dz * 3 + dy * 9;
		}

		glm::ivec3 origin;
		bool failed_to_lock = false;
		std::shared_ptr<Chunk> chunks[3*3*3];

	public:

		WorldRenderView(World& world, std::shared_ptr<Chunk>& center, Direction directions);

		bool failed() const;
		Block getBlock(int x, int y, int z);

};

struct BlockPlaneView {

	uint16_t* west;
	uint16_t* east;
	uint16_t* down;
	uint16_t* up;
	uint16_t* north;
	uint16_t* south;

};

struct ChunkPlane {

	uint16_t faces[Chunk::size * Chunk::size];

	uint16_t& at(int alpha, int beta) {
		return faces[beta + alpha * Chunk::size];
	}

};

struct ChunkFaceBuffer {

	private:

		static constexpr size_t planes_along_axis = Chunk::size;
		static constexpr size_t planes_per_axis = 2;
		static constexpr size_t size = 3 /* 3D */ * planes_along_axis * planes_per_axis;

		ChunkPlane* buffer = nullptr;

		ChunkPlane& get(int value, int offset) {
			return buffer[planes_per_axis * value + offset];
		}

	public:

		ChunkFaceBuffer() {
			this->buffer = new ChunkPlane[size];
		}

		~ChunkFaceBuffer() {
			delete[] this->buffer;
		}

		void clear() {
			memset(this->buffer, 0, size * sizeof(ChunkPlane));
		}

		ChunkPlane& getX(int x, int offset) {
			return get(x, offset);
		}

		ChunkPlane& getY(int y, int offset) {
			return get(y, offset + 1 * planes_along_axis * planes_per_axis);
		}

		ChunkPlane& getZ(int z, int offset) {
			return get(z, offset + 2 * planes_along_axis * planes_per_axis);
		}

		BlockPlaneView getBlockView(int x, int y, int z) {
			BlockPlaneView view;

			view.west = &getX(x, 0).at(y, z);
			view.east = &getX(x, 1).at(y, z);
			view.down = &getY(y, 0).at(x, z);
			view.up = &getY(y, 1).at(x, z);
			view.north = &getZ(z, 0).at(x, y);
			view.south = &getZ(z, 1).at(x, y);

			return view;
		}



};

class ChunkRenderPool {

	private:

		class UpdateRequest {

			private:

				glm::ivec3 pos;
				Timer timer;

			public:

				UpdateRequest(glm::ivec3 pos);
				glm::ivec3 unpack() const;

		};

		bool stop = false;
		std::mutex mutex;
		std::unordered_set<glm::ivec3> set;
		std::queue<UpdateRequest> high_queue;
		std::queue<UpdateRequest> low_queue;
		std::condition_variable condition;
		std::vector<std::thread> workers;

		WorldRenderer& renderer;
		RenderSystem& system;
		World& world;

		/// returns a new job from the queues, assumes mutex is already locked
		std::pair<bool, glm::ivec3> pop();

		/// checks if there is any work to do
		bool empty();

		/// emit cube geometry into the given mesh vector
		void emitCube(std::vector<VertexTerrain>& mesh, float x, float y, float z, uint8_t r, uint8_t g, uint8_t b, BlockPlaneView faces);

		/// emit the mesh of the given chunk into the render system, @Note this method internally uses threading
		void emitChunk(ChunkFaceBuffer& buffer, std::vector<VertexTerrain>& mesh, std::shared_ptr<Chunk> chunk);

		/// the worker threads' main function
		void run();

	public:

		ChunkRenderPool(WorldRenderer& renderer, RenderSystem& system, World& world);

		/// add a chunk remesh request
		void push(glm::ivec3 chunk, bool important);

		/// wait for all pending jobs and free resources
		void close();

};
