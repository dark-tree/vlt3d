#pragma once

#include "external.hpp"
#include "util/timer.hpp"
#include "client/vertices.hpp"

class World;
class Chunk;
class RenderSystem;
class WorldRenderer;
class WorldRenderView;
class ChunkFaceBuffer;

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

		/// emit the mesh of the given chunk into the given vector
		void emitChunk(std::vector<VertexTerrain>& mesh, ChunkFaceBuffer& buffer, std::shared_ptr<Chunk> chunk);

		/// the worker threads' main function
		void run();

	public:

		ChunkRenderPool(WorldRenderer& renderer, RenderSystem& system, World& world);

		/// add a chunk remesh request
		void push(glm::ivec3 chunk, bool important);

		/// wait for all pending jobs and free resources
		void close();

};