#pragma once

#include "external.hpp"

class MemoryRange {

	public:

		READONLY VkMappedMemoryRange vk_range {};

	public:

		MemoryRange(VkDeviceMemory vk_memory, uint64_t offset, uint64_t size) {
			vk_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			vk_range.memory = vk_memory;
			vk_range.offset = offset;
			vk_range.size = size;
		}

		void flush(VkDevice vk_device) const {
			vkFlushMappedMemoryRanges(vk_device, 1, &vk_range);
		}

		void invalidate(VkDevice vk_device) const {
			vkInvalidateMappedMemoryRanges(vk_device, 1, &vk_range);
		}

};

class MemoryAccess {

	private:

		VkDeviceMemory vk_memory;
		VkDevice vk_device;
		uint64_t size;

	public:

		MemoryAccess(VkDevice vk_device, VkDeviceMemory vk_memory, uint64_t size)
		: vk_device(vk_device), vk_memory(vk_memory), size(size) {}

		MemoryRange range(uint64_t offset, uint64_t size) const {
			return {vk_memory, offset, size};
		}

	public:

		void* map() const {
			void* data;
			vkMapMemory(vk_device, vk_memory, 0, size, 0, &data);
			return data;
		}

		void map(void* src) const {
			void* dst = map();
			memcpy(dst, src, size);
		}

		void unmap() const {
			vkUnmapMemory(vk_device, vk_memory);
		}

		void flush() const {
			range(0, size).flush(vk_device);
		}

		void invalidate() const {
			range(0, size).invalidate(vk_device);
		}

};

class Memory {

	public:

		READONLY VkDeviceMemory vk_memory;
		READONLY VkDevice vk_device;

	public:

		Memory(VkDeviceMemory vk_memory, VkDevice vk_device)
		: vk_memory(vk_memory), vk_device(vk_device) {}

		void close() const {
			vkFreeMemory(vk_device, vk_memory, nullptr);
		}

		MemoryAccess access(uint64_t size) const {
			return {vk_device, vk_memory, size};
		}

};

class MemoryInfo {

	public:

		READONLY VkPhysicalDeviceMemoryProperties vk_properties;
		READONLY VkPhysicalDevice vk_physical_device;
		READONLY VkDevice vk_device;

	public:

		MemoryInfo(VkPhysicalDevice vk_physical_device, VkDevice vk_device)
		: vk_physical_device(vk_physical_device), vk_device(vk_device) {
			vkGetPhysicalDeviceMemoryProperties(vk_physical_device, &vk_properties);
		}

	public:

		uint32_t find(int32_t filter, VkMemoryPropertyFlags flags) const {
			for (uint32_t i = 0; i < vk_properties.memoryTypeCount; i ++) {
				if ((filter & (1 << i)) && (vk_properties.memoryTypes[i].propertyFlags & flags) == flags) {
					return i;
				}
			}

			throw std::runtime_error("Failed to find matching memory!");
		}

		Memory allocate(const VkMemoryRequirements& requirements, VkMemoryPropertyFlags flags) const {

			VkMemoryAllocateInfo alloc_info {};
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize = requirements.size;
			alloc_info.memoryTypeIndex = find(requirements.memoryTypeBits, flags);

			VkDeviceMemory memory;

			if (vkAllocateMemory(vk_device, &alloc_info, nullptr, &memory) != VK_SUCCESS) {
				throw std::runtime_error("vkAllocateMemory: Failed to allocate memory!");
			}

			return {memory, vk_device};
		}

};
