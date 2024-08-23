
#include "buffer.hpp"
#include "allocator.hpp"
#include "command/recorder.hpp"
#include "client/renderer.hpp"

/*
 * Buffer
 */

BufferInfo Buffer::getHostBufferBuilder(size_t capacity, VkBufferUsageFlags usage) {
	BufferInfo builder {capacity, usage};

	builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	builder.preferred(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
	builder.hint(VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

	return builder;
}

BufferInfo Buffer::getDeviceBufferBuilder(size_t capacity, VkBufferUsageFlags usage) {
	BufferInfo builder {capacity, usage};

	builder.required(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	builder.hint(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	return builder;
}

Buffer::Buffer(VkBuffer vk_buffer, const MemoryAccess& memory)
: vk_buffer(vk_buffer), memory(memory) {}

void Buffer::realloc(RenderSystem& system, const BufferInfo& info, NULLABLE CommandRecorder* recorder) {

	Buffer buffer = system.allocator.allocateBuffer(info);

	if (recorder) {
		recorder->copyBufferToBuffer(*this, buffer, info.getBufferInfo().size);
	}

	system.defer([*this] () mutable {
		close();
	});

	this->vk_buffer = buffer.vk_buffer;
	this->memory = buffer.memory;

}

MemoryAccess& Buffer::access() {
	return memory;
}

void Buffer::close() {
	memory.closeBuffer(vk_buffer);
}

void Buffer::setDebugName(const Device& device, const char* name) const {
	VulkanDebug::name(device.vk_device, VK_OBJECT_TYPE_BUFFER, vk_buffer, name);
}

/*
 * BasicBuffer
 */

void BasicBuffer::reallocate(RenderSystem& system, size_t capacity) {
	system.defer([*this] () mutable {
		close();
	});

	this->buffer = system.allocator.allocateBuffer(Buffer::getDeviceBufferBuilder(capacity, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	this->staged = system.allocator.allocateBuffer(Buffer::getHostBufferBuilder(capacity, VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
	this->map = staged.access().map();
	this->capacity = capacity;

	updateDebugName();
}

size_t BasicBuffer::encompass(size_t target) {
	size_t size = this->capacity;

	while (size < target) {
		size *= 2;
	}

	return size;
}

void BasicBuffer::updateDebugName() const {
	#if !defined(NDEBUG)
	if (debug_device && debug_name) {
		std::string staged_name {"Staged "};
		staged_name += debug_name;

		if (capacity > 0) {
			VulkanDebug::name(debug_device, VK_OBJECT_TYPE_BUFFER, staged.vk_buffer, staged_name.c_str());
			VulkanDebug::name(debug_device, VK_OBJECT_TYPE_BUFFER, buffer.vk_buffer, debug_name);
		}
	}
	#endif
}

BasicBuffer::BasicBuffer(RenderSystem& system, size_t initial)
: capacity(0), count(0) {
	if (initial > 0) {
		reallocate(system, initial);
	}
}

void BasicBuffer::reserveToFit(RenderSystem& system, size_t bytes) {
	if (bytes > capacity && bytes != 0) {
		reallocate(system, encompass(bytes));
	}
}

size_t BasicBuffer::expand(RenderSystem& system) {
	size_t expansion = capacity;
	reallocate(system, expansion * 2);
	return expansion;
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

void BasicBuffer::setDebugName(const Device& device, const char* name) {
	#if !defined(NDEBUG)
	this->debug_device = device.vk_device;
	this->debug_name = name;
	updateDebugName();
	#endif
}

UnifiedBuffer::UnifiedBuffer(RenderSystem& system, size_t buffer_size, size_t staged_size)
: arena(buffer_size / 16, 512), offset(0), buffer_size(buffer_size), staged_size(staged_size) {
	this->buffer = system.allocator.allocateBuffer(Buffer::getDeviceBufferBuilder(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
	this->staged = system.allocator.allocateBuffer(Buffer::getHostBufferBuilder(staged_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
	this->map = staged.access().map();
}

std::mutex unified_mutex;

void UnifiedBuffer::upload(CommandRecorder& recorder) {

	std::lock_guard lock {unified_mutex};

	for (Transfer transfer : transfers) {
		recorder.copyBufferToBuffer(buffer, staged, transfer.length, transfer.target, transfer.offset);
	}

	offset = 0;
	transfers.clear();
}