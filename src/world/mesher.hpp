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
		void emitCube(std::vector<VertexTerrain>& mesh, float x, float y, float z, uint8_t r, uint8_t g, uint8_t b, bool west, bool east, bool down, bool up, bool north, bool south, BakedSprite bottom_sprite, BakedSprite top_sprite, BakedSprite side_sprite);

		/// emit the mesh of the given chunk into the render system, @Note this method internally uses threading
		void emitChunk(std::vector<VertexTerrain>& mesh, std::shared_ptr<Chunk> chunk);

		/// the worker threads' main function
		void run();

	public:

		ChunkRenderPool(WorldRenderer& renderer, RenderSystem& system, World& world);

		/// add a chunk remesh request
		void push(glm::ivec3 chunk, bool important);

		/// wait for all pending jobs and free resources
		void close();

};
