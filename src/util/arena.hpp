#pragma once

#include "external.hpp"
#include "util/exception.hpp"
#include "util/bits.hpp"

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

class LinearArena {

	public:

		template <std::unsigned_integral T>
		struct Brick {

			private:

				T value = std::numeric_limits<T>::max();

				/// Find index of first set bit in the number
				static inline int firstSetBitIndex(T value) {
					int index = 0;

					while (value >>= 1) {
						index ++;
					}

					return index;
				}

			public:

				Brick() = default;

				static constexpr size_t capacity = Bits::width<T>();

				/// Unset the ith bit
				int allocate() {
					int bit = firstSetBitIndex(value);
					value &= ~(1 << bit);
					return bit;
				}

				/// Set the ith bit
				void free(int bit) {
					value |= (1 << bit);
				}

				/// Check if the Brick has any set bits
				bool hasFree() const {
					return value;
				}

				/// Check if the Brick has any unset bits
				bool hasAllocated() const {
					return value != std::numeric_limits<T>::max();
				}

				/// Get the highest currently allocated value
				long top() const {
					return firstSetBitIndex(static_cast<T>(~value));
				}

		};

		template <std::unsigned_integral T, size_t count>
		struct Block {

			private:

				size_t allocated = 0;
				std::array<Brick<T>, count> bricks;

			public:

				Block() = default;

				static constexpr size_t stride = Brick<T>::capacity;
				static constexpr size_t capacity = Brick<T>::capacity * count;

				/// Allocate new unused number, call only if numbers remain
				long allocate() {
					for (long i = 0; i < (long) count; i ++) {
						Brick<T>& brick = bricks[i];

						if (brick.hasFree()) {
							allocated ++;
							return i * stride + brick.allocate();
						}
					}

					return -1;
				}

				/// Free the allocated number
				void free(long value) {
					long div = value / stride;
					long bit = value % stride;

					allocated --;
					bricks[div].free(bit);
				}

				/// Check how many more number can be allocated from this block
				long remaining() const {
					return capacity - allocated;
				}

				/// Check if the block doesn't contain any allocation
				bool hasAllocated() const {
					return allocated > 0;
				}

				/// Get the highest currently allocated value
				long top() const {
					for (long i = count - 1; i >= 0; i --) {
						auto& brick = bricks[i];

						if (brick.hasAllocated()) {
							return i * stride + brick.top();
						}
					}

					return -1;
				}

		};

	private:

		using StorageBlock = Block<uint32_t, 256>;

		mutable std::mutex mutex;
		std::list<StorageBlock> blocks;

	public:

		static constexpr long failed = -1;
		static constexpr size_t stride = StorageBlock::capacity;

		/// Allocates a new unique number, if no numbers are left returns -1
		long allocate() {
			std::lock_guard lock {mutex};

			for (auto it = blocks.begin(); it != blocks.end(); it ++) {
				if (it->remaining() > 0) {
					return std::distance(blocks.begin(), it) * stride + it->allocate();
				}
			}

			return -1;
		}

		/// Free the allocated number
		void free(long value) {
			std::lock_guard lock {mutex};

			long div = value / stride;
			long bit = value % stride;

			for (auto it = blocks.begin(); it != blocks.end(); it ++) {
				if (div == std::distance(blocks.begin(), it)) {
					it->free(bit);
				}
			}
		}

		/// Expand the allocation pool
		void expand() {
			std::lock_guard lock {mutex};
			blocks.emplace_back();
		}

		/// Get the highest currently allocated value
		long top() const {
			std::lock_guard lock {mutex};
			int i = blocks.size() - 1;

			for (auto it = blocks.rbegin(); it != blocks.rend(); it ++) {
				if (it->hasAllocated()) {
					return i * stride + it->top();
				}

				i --;
			}

			return -1;
		}

		long capacity() const {
			return blocks.size() * StorageBlock::capacity;
		}

		long remaining() const {
			std::lock_guard lock {mutex};
			long free = 0;

			for (auto it = blocks.begin(); it != blocks.end(); it ++) {
				free += it->remaining();
			}

			return free;
		}

};
