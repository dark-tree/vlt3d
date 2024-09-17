#pragma once

#include "external.hpp"
#include "chunk.hpp"

class ChunkColumn {

	private:

		int max_loaded_chunk = 0;
		int min_loaded_chunk = 0;

		ankerl::unordered_dense::map<int, std::shared_ptr<Chunk>> chunks;

	public:

		/// Get chunk or nullptr, requires external synchronization
		std::weak_ptr<Chunk> get(int cy);

		/// Replace or add a new chunk to the column
		void emplace(Chunk* chunk);

		/// Check if the column contains no chunks
		bool empty() const;

		/// Remove chunks outside the given max_distance
		void update(int max_distance, int camera_y);

		/// Check if the column contains chunk with given y
		bool contains(int cy) const;

};
