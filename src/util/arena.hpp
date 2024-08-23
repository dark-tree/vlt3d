#pragma once

#include "external.hpp"
#include "util/exception.hpp"

class AllocationBlock {

	private:

		struct Comparator {
			bool operator() (const AllocationBlock* lhs, const AllocationBlock* rhs) const {
				return lhs->offset < rhs->offset;
			}
		};

		enum State {
			USED      = 0,
			FREE      = 1,
			RECLAIMED = 2
		};

		State state;
		size_t offset;
		size_t length;

		AllocationBlock* left;
		AllocationBlock* right;

		void fastEraseFrom(std::set<AllocationBlock*, AllocationBlock::Comparator>& collection) {
			collection.erase(this);
		}

	private:

		friend class AllocationArena;

		AllocationBlock(size_t offset, size_t length, AllocationBlock* left, AllocationBlock* right) {
			this->offset = offset;
			this->length = length;
			this->left = left;
			this->right = right;
			this->state = FREE;

			if (left) {
				left->right = this;
			}

			if (right) {
				right->left = this;
			}
		}

		AllocationBlock* getNewBlock(std::set<AllocationBlock*, AllocationBlock::Comparator>& unused, size_t bytes, size_t margin = 0) {

			// check if we even fit, otherwise return an empty block
			if (bytes > length) {
				return nullptr;
			}

			const size_t bytes_left = length - bytes;

			// if less than margin is left in the parent block just return everything
			if (bytes_left <= margin) {
				fastEraseFrom(unused);
				return this;
			}

			// else return exactly the requested number
			size_t start = this->offset;
			this->length = bytes_left;
			this->offset += bytes;

			return new AllocationBlock {start, bytes, left, this};
		}

		void consolidate(std::set<AllocationBlock*, AllocationBlock::Comparator>& unused) {

			if (this->state == USED) {
				throw Exception {"Unable to consolidate used blocks!"};
			}

			// try merging `right` into `this`
			if (this->right && this->right->state) {
				AllocationBlock* right = this->right;
				size_t bytes = right->length;
				AllocationBlock* next = right->right;

				right->fastEraseFrom(unused);
				delete right;

				this->length += bytes;
				this->right = next;

				if (next) {
					next->left = this;
				}
			}

			// try merging `this` into `left`
			if (this->left && this->left->state) {
				AllocationBlock* left = this->left;
				size_t bytes = this->length;
				AllocationBlock* next = this->right;

				this->fastEraseFrom(unused);
				delete this;

				left->length += bytes;
				left->right = next;

				if (next) {
					next->left = left;
				}
			}
		}

		void setState(State state) {
			this->state = state;
		}

	public:

		size_t getOffset() const {
			return offset;
		}

		size_t getLength() const {
			return length;
		}

};

class AllocationArena {

	private:

		size_t margin, total;
		std::set<AllocationBlock*, AllocationBlock::Comparator> unused;

	public:

		struct Range {

			size_t start;
			size_t end;

		};

		struct Stats {

			size_t total;
			size_t free;
			size_t reclaimed;
			size_t used;
			size_t blocks;

			std::vector<Range> ranges;

		};

	public:

		AllocationArena() = default;
		AllocationArena(size_t bytes, size_t margin) {
			this->margin = margin;
			this->total = 0;

			expand(bytes);
		}

		AllocationBlock* allocate(size_t bytes) {
			for (auto it = unused.begin(); it != unused.end(); it ++) {
				if (AllocationBlock* allocation = (*it)->getNewBlock(unused, bytes, margin)) {
					allocation->setState(AllocationBlock::USED);
					return allocation;
				}
			}

			return nullptr;
		}

		void free(AllocationBlock* block) {
			block->setState(AllocationBlock::RECLAIMED);
			unused.insert(block);

			block->consolidate(unused);
		}

		void close() {
			size_t freed = 0;

			for (AllocationBlock* block : unused) {
				freed += block->length;
				delete block;
			}

			if (freed > total) {
				logger::error("Arena freed more memory that it was given! It's super effective!");
			}

			size_t leaked = total - freed;

			if (leaked > 0) {
				logger::warn("Arena leaked ", leaked, " bytes of memory, some blocks are still in use!");
			}
		}

		void expand(size_t bytes) {
			this->unused.insert(new AllocationBlock {this->total, bytes, nullptr, nullptr});
			this->total += bytes;
		}

		Stats getStats() const {

			Stats info {};
			info.used = total;
			info.total = total;

			for (AllocationBlock* block : unused) {
				if (block->state) {
					info.blocks ++;
					info.ranges.push_back({block->offset, block->offset + block->length});

					if (block->state == AllocationBlock::FREE) info.free += block->length;
					if (block->state == AllocationBlock::RECLAIMED) info.reclaimed += block->length;
				}
			}

			info.used -= info.reclaimed;
			info.used -= info.free;

			return info;

		}

};
