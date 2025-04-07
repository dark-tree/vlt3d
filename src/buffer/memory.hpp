#pragma once

#include "external.hpp"

extern std::unordered_map<VkBuffer, std::string> buffers;
extern std::unordered_map<VkImage, std::string> images;

class MemoryMap {

	private:

		VmaAllocator vma_allocator;
		VmaAllocation vma_allocation;
		uint8_t* pointer;

	public:

		class View {

			private:

				size_t offset;
				MemoryMap& map;

			public:

				View(MemoryMap& map)
				: offset(0), map(map) {}

				void write(const void* data, size_t bytes) {
					if (bytes > 0) {
						map.write(data, bytes, offset);
						map.flush(offset, bytes);
						offset += bytes;
					}
				}

				void read(void* data, size_t bytes) {
					if (bytes > 0) {
						map.invalidate(offset, bytes);
						map.read(data, bytes, offset);
						offset += bytes;
					}
				}

		};

	public:

		MemoryMap() = default;
		MemoryMap(VmaAllocator vma_allocator, VmaAllocation vma_allocation, void* pointer)
		: vma_allocator(vma_allocator), vma_allocation(vma_allocation), pointer((uint8_t*) pointer) {}

		void invalidate(size_t offset = 0, size_t size = VK_WHOLE_SIZE) {
			vmaInvalidateAllocation(vma_allocator, vma_allocation, offset, size);
		}

		void flush(size_t offset = 0, size_t size = VK_WHOLE_SIZE) {
			vmaFlushAllocation(vma_allocator, vma_allocation, offset, size);
		}

		void read(void* dst, size_t size, size_t offset = 0) {
			memcpy(dst, pointer + offset, size);
		}

		void write(const void* src, size_t size, size_t offset = 0) {
			memcpy(pointer + offset, src, size);
		}

		void unmap() {
			vmaUnmapMemory(vma_allocator, vma_allocation);
		}

		View getView() {
			return {*this};
		}

};

class MemoryAccess {

	public:

		READONLY VmaAllocator vma_allocator;
		READONLY VmaAllocation vma_allocation;

	public:

		MemoryAccess() = default;
		MemoryAccess(VmaAllocator vma_allocator, VmaAllocation vma_allocation)
		: vma_allocator(vma_allocator), vma_allocation(vma_allocation) {}

		VmaAllocationInfo getInfo() const {
			VmaAllocationInfo info;
			vmaGetAllocationInfo(vma_allocator, vma_allocation, &info);
			return info;
		}

		MemoryMap map() {
			void* pointer;
			vmaMapMemory(vma_allocator, vma_allocation, &pointer);
			return {vma_allocator, vma_allocation, pointer};
		}

		void closeBuffer(VkBuffer buffer) {
			buffers.erase(buffer);
			vmaDestroyBuffer(vma_allocator, buffer, vma_allocation);
		}

		void closeImage(VkImage image) {
			images.erase(image);
			vmaDestroyImage(vma_allocator, image, vma_allocation);
		}

};

class MemoryInfo {

	public:

		READONLY VkPhysicalDeviceMemoryProperties vk_properties;
		READONLY VkDevice vk_device;

	public:

		MemoryInfo() = default;
		MemoryInfo(VkPhysicalDevice vk_physical_device, VkDevice vk_device)
		: vk_device(vk_device) {
			vkGetPhysicalDeviceMemoryProperties(vk_physical_device, &vk_properties);
		}

		void printInfo(uint32_t index) const {
			VkMemoryType type = vk_properties.memoryTypes[index];
			VkMemoryHeap heap = vk_properties.memoryHeaps[type.heapIndex];

			VkMemoryPropertyFlags tf = type.propertyFlags;
			VkMemoryHeapFlags hf = heap.flags;

			printf("Memory #%u attributes:\n", index);

			if (tf & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) printf(" [T] VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT\n");
			if (tf & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) printf(" [T] VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT\n");
			if (tf & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) printf(" [T] VK_MEMORY_PROPERTY_HOST_COHERENT_BIT\n");
			if (tf & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) printf(" [T] VK_MEMORY_PROPERTY_HOST_CACHED_BIT\n");
			if (tf & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) printf(" [T] VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT\n");
			if (tf & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) printf(" [T] VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD\n");
			if (tf & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) printf(" [T] VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD\n");
			if (tf & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV) printf(" [T] VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV\n");
			if (hf & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) printf(" [H] VK_MEMORY_HEAP_DEVICE_LOCAL_BIT\n");
			if (hf & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) printf(" [H] VK_MEMORY_HEAP_MULTI_INSTANCE_BIT\n");

			printf(" [S] From heap #%d of size %lu MiB\n", type.heapIndex, heap.size / 1024 / 1024);
		}

	public:

		void print() const {

			printf("There are %d memory types and %d memory heaps\n", vk_properties.memoryTypeCount, vk_properties.memoryHeapCount);

			for (uint32_t i = 0; i < vk_properties.memoryTypeCount; i ++) {
				printf("\n");
				printInfo(i);
			}
		}

};
