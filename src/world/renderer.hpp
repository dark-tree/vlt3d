#pragma once

#include "chunk.hpp"
#include "world.hpp"
#include "client/vertices.hpp"
#include "client/renderer.hpp"
#include "command/recorder.hpp"
#include "client/immediate.hpp"

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

		// the thread pool used for meshing
		TaskPool pool;

		struct ChunkBuffer {
			glm::ivec3 pos;
			BasicBuffer buffer;

			ChunkBuffer(RenderSystem& system, glm::ivec3 pos, const std::vector<Vertex3D>& mesh);

			/// draw this buffer
			void draw(CommandRecorder& recorder);
		};

		/// emit cube geometry into the given mesh vector
		static void emitCube(std::vector<Vertex3D>& mesh, float x, float y, float z, float r, float g, float b, bool up, bool down, bool north, bool south, bool west, bool east, BakedSprite sprite);

		/// erase vertex buffer and mark it for deletion
		void eraseBuffer(RenderSystem& system, glm::ivec3 pos);

		/// emit the mesh of the given chunk into the render system, @Note this method internally uses threading
		void emitMesh(RenderSystem& system, const Atlas& atlas, World& world, std::shared_ptr<Chunk> chunk);

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

		/// Must be called before draw, collects chunks to remesh from the world and uploads pending vertex data
		void prepare(World& world, RenderSystem& system, CommandRecorder& recorder);

		/// Render all the chunk buffers, both static and just uploaded
		void draw(CommandRecorder& recorder);

		/// Submit a buffer, the pointer will be managed by this class
		void submitChunk(ChunkBuffer* chunk);

		/// Discard the chunk at the given coordinates
		void eraseChunk(glm::ivec3 pos);

		/// Discards all chunks that are further away than the given radius
		void eraseOutside(glm::ivec3 origin, float radius);

};
