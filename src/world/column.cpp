
#include "column.hpp"

std::weak_ptr<Chunk> ChunkColumn::get(int cy) {
	auto it = chunks.find(cy);

	if (it == chunks.end()) {
		return {};
	}

	return it->second;
}

void ChunkColumn::emplace(Chunk* chunk) {
	bool first = empty();
	int key = chunk->pos.y;
	chunks[key].reset(chunk);

	if (first || key > max_loaded_chunk) {
		max_loaded_chunk = key;
	}

	if (first || key < min_loaded_chunk) {
		min_loaded_chunk = key;
	}
}

bool ChunkColumn::empty() const {
	return chunks.empty();
}

void ChunkColumn::update(int max_distance, int camera_y) {
	int lower = std::abs(min_loaded_chunk - camera_y);
	int upper = std::abs(max_loaded_chunk - camera_y);

	if (lower > max_distance || upper > max_distance) {
		if (chunks.empty()) {
			return;
		}

		// we will search for the new max and min chunk
		int min = chunks.values()[0].first;
		int max = min;

		for (auto it = chunks.begin(); it != chunks.end();) {
			if (std::abs(it->first - camera_y) >= max_distance) {
				it = chunks.erase(it);
				continue;
			}

			if (it->first < min) min = it->first;
			if (it->first > max) max = it->first;

			std::advance(it, 1);
		}

		max_loaded_chunk = max;
		min_loaded_chunk = min;
	}
}

bool ChunkColumn::contains(int cy) const {
	if (cy > max_loaded_chunk) return false;
	if (cy < min_loaded_chunk) return false;

	return chunks.contains(cy);
}