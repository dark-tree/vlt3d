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
extern std::atomic_int world_frustum_count;
extern std::atomic_int world_occlusion_count;

// move this somewhere else?
template <typename T>
class DoubleBuffered {

	private:

		std::vector<T> front;
		std::vector<T> back;

	public:

		void swap() {
			front.clear();
			std::swap(front, back);
		}

		std::vector<T>& read() {
			return front;
		}

		std::vector<T>& write() {
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

		RenderSystem& system;
		World& world;
		ChunkRenderPool mesher;
		std::vector<int> allocations;

		struct ChunkBuffer {
			glm::ivec3 pos;
			long count, identifier;
			BasicBuffer buffer;

			ChunkBuffer(RenderSystem& system, glm::ivec3 pos, const std::vector<VertexTerrain>& mesh);

			/// draw this buffer unconditionally
			void draw(QueryPool& pool, CommandRecorder& recorder);

			/// dispose of this buffer as soon as it's valid to do so
			void dispose(RenderSystem& system);

			/// get the position of the chunk in world coordinates
			glm::vec3 getOffset() const;

			/// get the occlusion test result form the previous frame
			bool getOcclusion(QueryPool& pool) const;
		};

		/// erase vertex buffer and mark it for deletion
		void eraseBuffer(glm::ivec3 pos);

	private:

		// makes sure nothing gets fucked when we submit
		// chunk meshes from multiple threads
		std::mutex submit_mutex;

		// a list of all the to be rendered chunk buffers, each frame copied
		// and sorted from closest to furthest away from the camera
		std::vector<std::pair<glm::ivec3, ChunkBuffer*>> relative;

		// this holds the meshes of chunks that did not change (at least from
		// a rendering stand point - no new mesh is yet submitted for them)
		ankerl::unordered_dense::map<glm::ivec3, ChunkBuffer*> buffers;

		// this holds the buffers that have the new meshes written
		// but are not yet uploaded (copied from staging to device)
		DoubleBuffered<ChunkBuffer*> awaiting;

		// this holds the list of chunks that should be erased from the `buffers` map,
		// either because they are outside view or because new buffer was created
		std::vector<glm::ivec3> erasures;

	public:

		WorldRenderer(RenderSystem& system, World& world);

		/// Must be called before draw, collects chunks to remesh from the world and uploads pending vertex data
		void prepare(CommandRecorder& recorder);

		/// Render all the chunk buffers, both static and just uploaded
		void draw(CommandRecorder& recorder, Frustum& frustum, Camera& camera);

		/// Submit a buffer, mesh can be empty
		void submitChunk(glm::ivec3 pos, std::vector<VertexTerrain>& mesh);

		/// Discard the chunk at the given coordinates
		void eraseChunk(glm::ivec3 pos);

		/// Discards all chunks that are further away than the given radius
		void eraseOutside(glm::ivec3 origin, float radius);

		/// Discards all chunks and marks all resources for deletion
		void close();
};
