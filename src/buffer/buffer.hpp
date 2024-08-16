#pragma once

#include "external.hpp"
#include "setup/device.hpp"
#include "memory.hpp"

class Allocator;
class CommandRecorder;
class RenderSystem;

class Buffer {

	public:

		READONLY VkBuffer vk_buffer;
		READONLY MemoryAccess memory;

	public:

		Buffer() = default;
		Buffer(VkBuffer vk_buffer, const MemoryAccess& memory);

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
		void reserve(RenderSystem& system, size_t bytes);

		/**
		 * Performs an unsafe (no bounds checking) buffer write make sure the buffer
		 * is big enough to house the given data using `reserve()` or `checkedWrite()`
		 */
		template <typename T>
		void write(T* data, size_t count) {
			this->bytes = count * sizeof(T);
			this->count = count;

			if (count > 0) {
				map.write(data, bytes);
				map.flush();
			}
		}

		/**
		 * Helper method that combines `reserve()`
		 * and `write()` into a single call
		 */
		template <typename T>
		void checkedWrite(RenderSystem& system, T* data, size_t count) {
			reserve(system, count * sizeof(T));
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
