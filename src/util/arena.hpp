#pragma once

#include "external.hpp"
#include "util/exception.hpp"

class AllocationBlock {

	private:

		bool free;
		size_t offset;
		size_t length;

		AllocationBlock* left;
		AllocationBlock* right;

		void fastEraseFrom(std::vector<AllocationBlock*>& collection) {
			auto it = std::find(collection.begin(), collection.end(), this);
			*it = std::move(collection.back());
			collection.pop_back();
		}

	private:

		friend class AllocationArena;

		AllocationBlock(size_t offset, size_t length, AllocationBlock* left, AllocationBlock* right) {
			this->offset = offset;
			this->length = length;
			this->left = left;
			this->right = right;
			this->free = true;

			if (left) {
				left->right = this;
			}

			if (right) {
				right->left = this;
			}
		}

		AllocationBlock* getNewBlock(std::vector<AllocationBlock*>& unused, size_t bytes, size_t margin = 0) {

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

		void consolidate(std::vector<AllocationBlock*>& unused) {

			if (!this->free) {
				throw Exception {"Unable to consolidate non-free blocks!"};
			}

			// try merging `right` into `this`
			if (this->right && this->right->free) {
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
			if (this->left && this->left->free) {
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

		void setFree(bool flag) {
			this->free = flag;
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
		std::vector<AllocationBlock*> unused;

	public:

		AllocationArena(size_t bytes, size_t margin) {
			this->margin = margin;
			this->total = bytes;
			unused.push_back(new AllocationBlock {0, bytes, nullptr, nullptr});
		}

		AllocationBlock* allocate(size_t bytes) {
			for (auto it = unused.begin(); it != unused.end(); it ++) {
				if (AllocationBlock* allocation = (*it)->getNewBlock(unused, bytes, margin)) {
					allocation->setFree(false);
					return allocation;
				}
			}

			return nullptr;
		}

		void free(AllocationBlock* block) {
			block->setFree(true);
			unused.push_back(block);
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

};
