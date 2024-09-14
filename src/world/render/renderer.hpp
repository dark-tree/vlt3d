#pragma once

#include "world/chunk.hpp"
#include "world/world.hpp"
#include "mesher.hpp"
#include "client/vertices.hpp"
#include "client/renderer.hpp"
#include "command/recorder.hpp"
#include "client/immediate.hpp"
#include "client/frustum.hpp"
#include "pool.hpp"

extern std::atomic_int world_vertex_count;
extern std::atomic_int world_chunk_count;
extern std::atomic_int world_visible_count;
extern std::atomic_int world_occlusion_count;

// move this somewhere else?
template <typename T>
class DoubleBuffered {

	private:

		T front;
		T back;

	public:

		void swap() {
			front.clear();
			std::swap(front, back);
		}

		T& read() {
			return front;
		}

		T& write() {
			return back;
		}

};

/**
 * Method used for rendering a world,
 * The general operation looks like this:
 * @verbatim
 *
 * 1. Call prepare()
 *    1) This consumes a list of updates from the World using `consumeUpdates()`
 *    2) Each update (remesh) is started on thread pool by calling `emitMesh()`
 *       Once each mesh is done a `submitChunk()` is called from the worker thread
 *    3) A list of recently submitted chunks is iterated and each new chunk is now
 *       uploaded to the GPU
 *
 * 2. Call draw()
 *    1) All non-changed chunks are now rendered
 *    2) Draw all the new chunks that are now uploaded to the GPU
 */
class WorldRenderer {

	private:

		// a unique chunk remesh identifier, incremented by one, used to compare submissions for the same chunk
		uint64_t unique_stamp;
		RenderSystem& system;
		World& world;
		ChunkRenderPool mesher;
		std::vector<int> allocations;

		class ChunkBuffer {

			private:

				uint64_t stamp;
				std::array<uint32_t, 7> region_begin, region_count;

			public:

				glm::ivec3 pos;
				long identifier;
				BasicBuffer buffer;

			public:

				ChunkBuffer(RenderSystem& system, glm::ivec3 pos, const MeshEmitterSet& emitters, uint64_t stamp);

				/// draw this buffer unconditionally
				void draw(QueryPool& pool, CommandRecorder& recorder, glm::vec3 camera_pos);

				/// dispose of this buffer as soon as it's valid to do so
				void dispose(RenderSystem& system);

				/// get the position of the chunk in world coordinates
				glm::vec3 getOffset() const;

				/// get the occlusion test result form the previous frame
				bool getOcclusion(QueryPool& pool) const;

				/// Get the total number of vertices in this chunk
				long getCount() const;

				/// If two version of a chunk exist this method can be used to check which one should be used
				bool shouldReplace(ChunkBuffer* chunk) const;

		};

		/// replace a chunk in the buffer map with correct chunk cleanup
		void replaceChunk(glm::ivec3 pos, NULLABLE ChunkBuffer* chunk);

	private:

		// makes sure nothing gets fucked when we submit
		// chunk meshes from multiple threads
		std::mutex submit_mutex;

		// a list of all the to be rendered chunk buffers, each frame copied
		// and sorted from closest to furthest away from the camera
		std::vector<std::pair<float, ChunkBuffer*>> relative;

		// this holds the meshes of chunks that did not change (at least from
		// a rendering stand point - no new mesh is yet submitted for them)
		ankerl::unordered_dense::map<glm::ivec3, ChunkBuffer*> buffers;

		// this holds the buffers that have the new meshes written
		// but are not yet uploaded (copied from staging to device)
		DoubleBuffered<std::unordered_map<glm::ivec3, ChunkBuffer*>> awaiting;

		// this holds the list of chunks that should be erased from the `buffers` map,
		// either because that was requested manually or because they are now outside view distance
		std::vector<glm::ivec3> erasures;

	public:

		WorldRenderer(RenderSystem& system, World& world);

		/// Must be called before draw, collects chunks to remesh from the world and uploads pending vertex data
		void prepare(CommandRecorder& recorder);

		/// Render all the chunk buffers, both static and just uploaded
		void draw(CommandRecorder& recorder, Frustum& frustum, Camera& camera);

		/// Submit a buffer, mesh can be empty
		void submitChunk(glm::ivec3 pos, const MeshEmitterSet& emitters, uint64_t stamp);

		/// Discard the chunk at the given coordinates
		void eraseChunk(glm::ivec3 pos);

		/// Discards all chunks that are further away than the given radius
		void eraseOutside(glm::ivec3 origin, float radius);

		/// Discards all chunks and marks all resources for deletion
		void close();
};
