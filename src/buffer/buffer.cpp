
#include "buffer.hpp"
#include "allocator.hpp"

Buffer::Buffer(VkBuffer vk_buffer, const MemoryAccess& memory)
: vk_buffer(vk_buffer), memory(memory) {}

MemoryAccess& Buffer::access() {
	return memory;
}

void Buffer::close() {
	memory.closeBuffer(vk_buffer);
}

void BasicBuffer::reallocate(size_t capacity) {
	BufferInfo buffer_builder {capacity, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
	buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

	close();
	buffer = allocator.allocateBuffer(buffer_builder);
	this->capacity = capacity;
	logger::info("Reallocated 3D simple buffer to ", capacity, " bytes");
}

size_t BasicBuffer::encompass(size_t target) {
	size_t size = this->capacity;

	while (size < target) {
		size *= 2;
	}

	return size;
}

BasicBuffer::BasicBuffer(Allocator& allocator, size_t initial)
: allocator(allocator), capacity(0), count(0) {
	reallocate(initial);
}

size_t BasicBuffer::getCount() const {
	return count;
}

const Buffer& BasicBuffer::getBuffer() const {
	return buffer;
}

void BasicBuffer::close() {
	if (capacity > 0) buffer.close();
}
