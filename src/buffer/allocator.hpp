#pragma once

#include "external.hpp"
#include "buffer.hpp"

class BufferInfo {

	private:

		READONLY VkMemoryPropertyFlags vk_required_flags;
		READONLY VkMemoryPropertyFlags vk_preferred_flags;
		READONLY VmaMemoryUsage vma_usage;
		READONLY VmaAllocationCreateFlags vma_flags;
		READONLY VkBufferUsageFlags vk_buffer_usage;
		READONLY VkSharingMode vk_sharing;
		READONLY uint32_t bytes;

	public:

		BufferInfo(uint32_t size, VkBufferUsageFlags usage)
		: vk_required_flags(0), vk_preferred_flags(0), vma_usage(VMA_MEMORY_USAGE_AUTO), vma_flags(0), vk_buffer_usage(usage), vk_sharing(VK_SHARING_MODE_EXCLUSIVE), bytes(size) {}

		BufferInfo()
		: BufferInfo(0, 0) {}

		VkBufferCreateInfo getBufferInfo() const {
			VkBufferCreateInfo create_info {};

			create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			create_info.size = bytes;
			create_info.usage = vk_buffer_usage;
			create_info.sharingMode = vk_sharing;

			return create_info;
		}

		VmaAllocationCreateInfo getAllocationInfo() const {
			VmaAllocationCreateInfo create_info {};

			create_info.flags = vma_flags;
			create_info.usage = vma_usage;
			create_info.preferredFlags = vk_preferred_flags;
			create_info.requiredFlags = vk_required_flags;

			return create_info;
		}

	public:

		inline BufferInfo& required(VkMemoryPropertyFlags flags) {
			vk_required_flags = flags;
			return *this;
		}

		inline BufferInfo& preferred(VkMemoryPropertyFlags flags) {
			vk_preferred_flags = flags;
			return *this;
		}

		inline BufferInfo& flags(VmaAllocationCreateFlagBits flags) {
			vma_flags = flags;
			return *this;
		}

		inline BufferInfo& hint(VmaMemoryUsage usage) {
			vma_usage = usage;
			return *this;
		}

		inline BufferInfo& usage(VkBufferUsageFlags usage) {
			vk_buffer_usage = usage;
			return *this;
		}

		inline BufferInfo& size(uint32_t size) {
			bytes = size;
			return *this;
		}

		inline BufferInfo& shared(bool flag = true) {
			vk_sharing = flag ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
			return *this;
		}

};

/**
 * A wrapper around the VMA library
 * @see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/index.html
 */
class Allocator {

	private:

		VmaAllocator vma_allocator;

	public:

		Allocator(Device& device, Instance& instance) {
			VmaVulkanFunctions functions = {};
			functions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
			functions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

			VmaAllocatorCreateInfo create_info {};
			create_info.physicalDevice = device.vk_physical_device;
			create_info.device = device.vk_device;
			create_info.pVulkanFunctions = &functions;
			create_info.instance = instance.vk_instance;
			create_info.vulkanApiVersion = VK_API_VERSION_1_0;

			vmaCreateAllocator(&create_info, &vma_allocator);
		}

		void close() {
			vmaDestroyAllocator(vma_allocator);
		}

	public:

		Buffer allocateBuffer(const BufferInfo& info) {
			VkBuffer buffer;
			VmaAllocation allocation;

			const VmaAllocationCreateInfo allocation_info = info.getAllocationInfo();
			const VkBufferCreateInfo buffer_info = info.getBufferInfo();

			if(vmaCreateBuffer(vma_allocator, &buffer_info, &allocation_info, &buffer, &allocation, nullptr) != VK_SUCCESS) {
				throw std::runtime_error {"vmaCreateBuffer: Failed to allocated buffer!"};
			}

			return {buffer, {vma_allocator, allocation}};
		}

};