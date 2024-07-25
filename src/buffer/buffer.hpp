#pragma once

#include "external.hpp"
#include "setup/device.hpp"
#include "memory.hpp"

class Allocator;

class Buffer {

	public:

		READONLY VkBuffer vk_buffer;
		READONLY MemoryAccess memory;

	public:

		Buffer() = default;
		Buffer(VkBuffer vk_buffer, const MemoryAccess& memory);

		MemoryAccess& access();
		void close();

};

/**
 * OpenGL style generic buffer that automatically resizes
 * this is a stop-gap solution as doing something like this
 * in vulkan is simply a crime but for now - its better than
 * creating a new buffer each frame
 */
class BasicBuffer {

	private:

		Allocator& allocator;
		Buffer buffer;
		size_t capacity, count;

		void reallocate(size_t capacity);
		size_t encompass(size_t target);

	public:

		BasicBuffer(Allocator& allocator, size_t initial);

		template <typename T>
		void write(T* data, size_t count) {
			size_t bytes = count * sizeof(T);
			this->count = count;

			if (bytes > capacity) {
				reallocate(encompass(bytes));
			}

			if (count > 0) {
				MemoryMap map = buffer.access().map();
				map.write(data, bytes);
				map.flush();
				map.unmap();
			}
		}

		size_t getCount() const;
		const Buffer& getBuffer() const;
		void close();

};
