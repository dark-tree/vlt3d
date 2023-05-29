#pragma once

#include "external.hpp"

class MemoryAccess {

	private:
	
		VkDeviceMemory vk_memory;
		VkDevice vk_device;
		uint64_t size;
		
	public:
	
		MemoryAccess(VkDevice vk_device, VkDeviceMemory vk_memory, uint64_t size)
		: vk_device(vk_device), vk_memory(vk_memory), size(size) {}

	public:
	
		void* map() {
			void* data;
			vkMapMemory(vk_device, vk_memory, 0, size, 0, &data);
			return data;
		}
		
		void unmap() {
			vkUnmapMemory(vk_device, vk_memory);
		}
		
		void write(void* src) {
			void* dst = map();
			memcpy(dst, src, size);
			unmap();
			
		}
	
//		void flush() {
//			vkFlushMappedMemoryRanges(vk_device, 1, &vk_memory);
//		}
//		
//		void invalidate() {
//			vkInvalidateMappedMemoryRanges(vk_device, 1, &vk_memory);
//		}

};

class Memory {

	public:
	
		READONLY VkDeviceMemory vk_memory;
		READONLY VkDevice vk_device;
		
	public:
	
		Memory(VkDeviceMemory vk_memory, VkDevice vk_device)
		: vk_memory(vk_memory), vk_device(vk_device) {}
	
		void close() {
			vkFreeMemory(vk_device, vk_memory, nullptr);
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
