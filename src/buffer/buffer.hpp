#pragma once

#include "external.hpp"
#include "setup/device.hpp"
#include "memory.hpp"
#include "util/arena.hpp"
#include "util/bits.hpp"

class Allocator;
class CommandRecorder;
class RenderSystem;
class BufferInfo;

class Buffer {

	public:

		READONLY VkBuffer vk_buffer;
		READONLY MemoryAccess memory;

	public:

		static BufferInfo getHostBufferBuilder(size_t capacity, VkBufferUsageFlags usage);
		static BufferInfo getDeviceBufferBuilder(size_t capacity, VkBufferUsageFlags usage);

	public:

		Buffer() = default;
		Buffer(VkBuffer vk_buffer, const MemoryAccess& memory);

		void realloc(RenderSystem& system, const BufferInfo& info, NULLABLE CommandRecorder* recorder);
		MemoryAccess& access();
		void close();

		void setDebugName(const Device& device, const char* name) const;

};

/**
 * OpenGL style generic vertex buffer that automatically resizes
 */
class BasicBuffer {

	private:

		Buffer buffer;
		Buffer staged;
		size_t capacity, count, bytes;
		MemoryMap map;

		#if !defined(NDEBUG)
		VkDevice debug_device = VK_NULL_HANDLE;
		const char* debug_name = nullptr;
		#endif

		void reallocate(RenderSystem& system, size_t capacity);
		size_t encompass(size_t target);

		void updateDebugName() const;

	public:

		BasicBuffer(RenderSystem& system, size_t initial);

		/**
		 * Relocates the buffer if that is needed to
		 * house the given number of bytes
		 */
		void reserveToFit(RenderSystem& system, size_t bytes);

		/**
		 * Relocates the buffer so that
		 * it is twice it's current capacity
		 */
		size_t expand(RenderSystem& system);

		/**
		 * Performs an unsafe (no bounds checking) buffer write make sure the buffer
		 * is big enough to house the given data using `reserve()` or `checkedWrite()`
		 */
		template <typename T>
		void write(T* data, size_t count, size_t offset = 0) {
			this->bytes = count * sizeof(T);
			this->count = count;

			if (count > 0) {
				map.write(data, bytes, offset);
				map.flush();
			}
		}

		/**
		 * Helper method that combines `reserveToFit()`
		 * and `write()` into a single call
		 */
		template <typename T>
		void checkedWrite(RenderSystem& system, T* data, size_t count) {
			reserveToFit(system, count * sizeof(T));
			write(data, count);
		}

		void upload(CommandRecorder& recorder);
		void draw(CommandRecorder& recorder);

	public:

		/// If a buffer is empty there is no need to call close()
		bool empty() const;

		/// Returns the number of vertices in this buffer, @Note consider using `draw()`
		size_t getCount() const;

		/// Returns the internal Buffer object, @Note consider using `draw()`
		const Buffer& getBuffer() const;

		/// Frees internal vulkan resources, consider doing it from `RenderSystem.defer()`
		void close();

		void setDebugName(const Device& device, const char* name);

};

extern std::mutex unified_mutex;

class UnifiedBuffer {

	public:

		struct Transfer {
			size_t offset;
			size_t length;
			size_t target;
		};

		size_t buffer_size;
		Buffer buffer;

		size_t staged_size;
		Buffer staged;

		std::vector<Transfer> transfers;
		AllocationArena arena;
		size_t offset;
		MemoryMap map;

	public:

		UnifiedBuffer() = default;
		UnifiedBuffer(RenderSystem& system, size_t buffer_size, size_t staged_size);

		template <typename T>
		AllocationBlock* write(T* data, size_t count) {
//			if (offset + count * sizeof(T) >= staged_size) {
//				this->staged.realloc(Buffer::getHostBufferBuilder(staged_size * 2, VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
//				this->map = staged.access().map();
//			}

			// i hate c++
			std::lock_guard lock {unified_mutex};

			size_t bytes = count * sizeof(T);
			map.write(data, bytes, offset);

			AllocationBlock* block = arena.allocate(Bits::adiv<size_t>(bytes, 16));

			transfers.emplace_back(offset, bytes, block->getOffset() * 16);
			offset += bytes;

//			if (block == nullptr) {
//				arena.expand(buffer.expand(system));
//				return write(system, data, count);
//			}

			return block;
		}

		void free(AllocationBlock* block) {
			arena.free(block);
		}

		void close() {
			buffer.close();
			arena.close();
		}

		void upload(CommandRecorder& recorder);

};