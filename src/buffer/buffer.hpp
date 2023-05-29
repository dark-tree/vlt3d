#pragma once

#include "external.hpp"
#include "setup/device.hpp"
#include "memory.hpp"

class Buffer {

	public:
	
		READONLY uint64_t size;
		READONLY VkBuffer vk_buffer;
		READONLY VkDevice vk_device;
		READONLY Memory memory;
	
	public:
	
		static VkMemoryRequirements getRequirements(VkDevice vk_device, VkBuffer vk_buffer) {
			VkMemoryRequirements requirements;
			vkGetBufferMemoryRequirements(vk_device, vk_buffer, &requirements);
			return requirements;
		}
		
		Buffer(uint64_t size, VkBuffer vk_buffer, Device& device, const Memory& memory)
		: size(size), vk_buffer(vk_buffer), vk_device(device.vk_device), memory(memory) {
			vkBindBufferMemory(vk_device, vk_buffer, memory.vk_memory, 0);
		}
		
	public:
		
		void close() {
			vkDestroyBuffer(vk_device, vk_buffer, nullptr);
			memory.close();
		}
		
		MemoryAccess access() {
			return {vk_device, memory.vk_memory, size};
		}
		
		static Buffer from(Device& device, uint64_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, bool shared = false) {
		
			VkBufferCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			create_info.size = size;
			create_info.usage = usage;
			create_info.sharingMode = shared ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
			
			VkBuffer buffer;

			if (vkCreateBuffer(device.vk_device, &create_info, nullptr, &buffer) != VK_SUCCESS) {
				throw std::runtime_error("vkCreateBuffer: Failed to create vertex buffer!");
			}
			
			return {size, buffer, device, device.memory.allocate(getRequirements(device.vk_device, buffer), flags)};
		}

};
