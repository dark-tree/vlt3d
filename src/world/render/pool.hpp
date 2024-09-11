#pragma once

#include "external.hpp"
#include "util/timer.hpp"
#include "client/vertices.hpp"
#include "world/view.hpp"

class World;
class Chunk;
class RenderSystem;
class WorldRenderer;
class ChunkFaceBuffer;

class ChunkRenderPool {

	private:

		class UpdateRequest {

			private:

				WorldView view;
				Timer timer;
				uint64_t stamp;

			public:

				UpdateRequest() = default;
				UpdateRequest(WorldView&& view, uint64_t stamp);
				glm::ivec3 origin() const;
				WorldView&& unpack();

				uint64_t getStamp() const;
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
		std::pair<bool, UpdateRequest> pop();

		/// checks if there is any work to do
		bool empty();

		/// emit the mesh of the given chunk into the given vector
		void emitChunk(std::vector<VertexTerrain>& mesh, ChunkFaceBuffer& buffer, WorldView& view, uint64_t stamp);

		/// the worker threads' main function
		void run();

	public:

		ChunkRenderPool(WorldRenderer& renderer, RenderSystem& system, World& world);

		/// add a chunk remesh request
		void push(WorldView&& view, bool important, uint64_t stamp);

		/// wait for all pending jobs and free resources
		void close();

};