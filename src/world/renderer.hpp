#pragma once

#include "chunk.hpp"
#include "world.hpp"
#include "mesher.hpp"
#include "client/vertices.hpp"
#include "client/renderer.hpp"
#include "command/recorder.hpp"
#include "client/immediate.hpp"
#include "client/frustum.hpp"

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

		struct ChunkBuffer {
			glm::ivec3 pos;
			BasicBuffer buffer;

			ChunkBuffer(RenderSystem& system, glm::ivec3 pos, const std::vector<Vertex3D>& mesh);

			/// draw this buffer
			void draw(CommandRecorder& recorder, Frustum& frustum);
		};

		/// erase vertex buffer and mark it for deletion
		void eraseBuffer(glm::ivec3 pos);

	private:

		// makes sure nothing gets fucked when we submit
		// chunk meshes from multiple threads
		std::mutex submit_mutex;

		// this holds the meshes of chunks that did not change (at least from
		// a rendering stand point - no new mesh is yet submitted for them)
		std::unordered_map<glm::ivec3, ChunkBuffer*> buffers;

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
		void draw(CommandRecorder& recorder, Frustum& frustum);

		/// Submit a buffer, mesh can be empty
		void submitChunk(glm::ivec3 pos, std::vector<Vertex3D>& mesh);

		/// Discard the chunk at the given coordinates
		void eraseChunk(glm::ivec3 pos);

		/// Discards all chunks that are further away than the given radius
		void eraseOutside(glm::ivec3 origin, float radius);

};
