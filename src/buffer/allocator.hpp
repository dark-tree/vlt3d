#pragma once

#include "external.hpp"
#include "buffer/buffer.hpp"
#include "buffer/image.hpp"
#include "setup/instance.hpp"

template <typename S>
class AllocationInfo {

	protected:

		READONLY VkMemoryPropertyFlags vk_required_flags;
		READONLY VkMemoryPropertyFlags vk_preferred_flags;
		READONLY VmaMemoryUsage vma_usage;
		READONLY VmaAllocationCreateFlags vma_flags;
		READONLY VkSharingMode vk_sharing;

	public:

		AllocationInfo()
		: vk_required_flags(0), vk_preferred_flags(0), vma_usage(VMA_MEMORY_USAGE_AUTO), vma_flags(0), vk_sharing(VK_SHARING_MODE_EXCLUSIVE) {}

		VmaAllocationCreateInfo getAllocationInfo() const {
			VmaAllocationCreateInfo create_info {};

			create_info.flags = vma_flags;
			create_info.usage = vma_usage;
			create_info.preferredFlags = vk_preferred_flags;
			create_info.requiredFlags = vk_required_flags;

			return create_info;
		}

	public:

		inline S& required(VkMemoryPropertyFlags flags) {
			vk_required_flags = flags;
			return (S&) *this;
		}

		inline S& preferred(VkMemoryPropertyFlags flags) {
			vk_preferred_flags = flags;
			return (S&) *this;
		}

		inline S& flags(VmaAllocationCreateFlagBits flags) {
			vma_flags = flags;
			return (S&) *this;
		}

		inline S& hint(VmaMemoryUsage usage) {
			vma_usage = usage;
			return (S&) *this;
		}

		inline S& shared(bool flag = true) {
			vk_sharing = flag ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
			return (S&) *this;
		}

};

class BufferInfo : public AllocationInfo<BufferInfo> {

	private:

		READONLY VkBufferUsageFlags vk_buffer_usage;
		READONLY size_t bytes;

	public:

		BufferInfo(size_t size, VkBufferUsageFlags usage)
		: AllocationInfo(), vk_buffer_usage(usage), bytes(size) {}

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

	public:

		inline BufferInfo& size(size_t size) {
			bytes = size;
			return *this;
		}

		inline BufferInfo& usage(VkBufferUsageFlags usage) {
			vk_buffer_usage = usage;
			return *this;
		}

};

class ImageInfo : public AllocationInfo<ImageInfo> {

	private:

		READONLY VkExtent3D vk_extent;
		READONLY VkFormat vk_format;
		READONLY VkImageTiling vk_tiling;
		READONLY VkImageUsageFlags vk_image_usage;
		READONLY VkSampleCountFlagBits vk_samples;

		VkImageType getImageType() const {
			if (vk_extent.depth == 1 && vk_extent.height == 1) {
				return VK_IMAGE_TYPE_1D;
			}

			if (vk_extent.depth == 1) {
				return VK_IMAGE_TYPE_2D;
			}

			return VK_IMAGE_TYPE_3D;
		}

	public:

		ImageInfo(size_t width, size_t height, VkFormat format, VkImageUsageFlags usage)
		: AllocationInfo(), vk_format(format), vk_tiling(VK_IMAGE_TILING_OPTIMAL), vk_image_usage(usage), vk_samples(VK_SAMPLE_COUNT_1_BIT) {
			size(width, height);
		}

		ImageInfo()
		: ImageInfo(0, 0, VK_FORMAT_R8G8B8_UINT, 0) {}

		VkImageCreateInfo getImageInfo() const {
			VkImageCreateInfo create_info {};

			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			create_info.imageType = getImageType();
			create_info.extent = vk_extent;
			create_info.mipLevels = 1;
			create_info.arrayLayers = 1;
			create_info.format = vk_format;
			create_info.tiling = vk_tiling;
			create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			create_info.usage = vk_image_usage;
			create_info.sharingMode = vk_sharing;
			create_info.samples = vk_samples;
			create_info.flags = 0;

			return create_info;
		}

	public:

		inline ImageInfo& size(size_t width, size_t height, size_t depth = 1) {
			vk_extent.width = width;
			vk_extent.height = height;
			vk_extent.depth = depth;
			return *this;
		}

		inline ImageInfo& format(VkFormat format) {
			vk_format = format;
			return *this;
		}

		inline ImageInfo& tiling(VkImageTiling tiling) {
			vk_tiling = tiling;
			return *this;
		}

		inline ImageInfo& usage(VkImageUsageFlags usage) {
			vk_image_usage = usage;
			return *this;
		}

		inline ImageInfo& samples(int samples) {
			vk_samples = (VkSampleCountFlagBits) samples;
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

		Allocator() = default;
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
				throw Exception {"Failed to allocated buffer!"};
			}

			return {buffer, {vma_allocator, allocation}};
		}

		Image allocateImage(const ImageInfo& info) {
			VkImage image;
			VmaAllocation allocation;

			const VmaAllocationCreateInfo allocation_info = info.getAllocationInfo();
			const VkImageCreateInfo image_info = info.getImageInfo();

			if(vmaCreateImage(vma_allocator, &image_info, &allocation_info, &image, &allocation, nullptr) != VK_SUCCESS) {
				throw Exception {"Failed to allocated buffer!"};
			}

			return {image, image_info.format, {vma_allocator, allocation}};
		}

};