
#include "buffer.hpp"
#include "allocator.hpp"
#include "command/recorder.hpp"
#include "client/renderer.hpp"

Buffer::Buffer(VkBuffer vk_buffer, const MemoryAccess& memory)
: vk_buffer(vk_buffer), memory(memory) {}

MemoryAccess& Buffer::access() {
	return memory;
}

void Buffer::close() {
	memory.closeBuffer(vk_buffer);
}

void BasicBuffer::reallocate(RenderSystem& system, size_t capacity) {
	BufferInfo staged_builder {capacity, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
	staged_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	staged_builder.preferred(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	staged_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
	staged_builder.hint(VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

	BufferInfo buffer_builder {capacity, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
	buffer_builder.required(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	buffer_builder.hint(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	system.defer([*this] () mutable {
		close();
	});

	this->buffer = system.allocator.allocateBuffer(buffer_builder);
	this->staged = system.allocator.allocateBuffer(staged_builder);
	this->map = staged.access().map();
	this->capacity = capacity;
}

size_t BasicBuffer::encompass(size_t target) {
	size_t size = this->capacity;

	while (size < target) {
		size *= 2;
	}

	return size;
}

BasicBuffer::BasicBuffer(RenderSystem& system, size_t initial)
: capacity(0), count(0) {
	if (initial > 0) {
		reallocate(system, initial);
	}
}

void BasicBuffer::reserve(RenderSystem& system, size_t bytes) {
	if (bytes > capacity && bytes != 0) {
		reallocate(system, encompass(bytes));
	}
}

void BasicBuffer::upload(CommandRecorder& recorder) {
	if (count > 0) {
		recorder.copyBufferToBuffer(this->buffer, this->staged, this->bytes);
	}
}

void BasicBuffer::draw(CommandRecorder& recorder) {
	if (count > 0) {
		recorder.bindBuffer(buffer).draw(count);
	}
}

bool BasicBuffer::empty() const {
	return capacity == 0;
}

size_t BasicBuffer::getCount() const {
	return count;
}

const Buffer& BasicBuffer::getBuffer() const {
	return buffer;
}

void BasicBuffer::close() {
	if (capacity > 0) {
		map.unmap();
		buffer.close();
		staged.close();
	}
}
