
#include "allocator.hpp"

/*
 * BufferInfo
 */

BufferInfo::BufferInfo(size_t size, VkBufferUsageFlags usage)
: AllocationInfo(), vk_buffer_usage(usage), bytes(size) {}

BufferInfo::BufferInfo()
: BufferInfo(0, 0) {}

VkBufferCreateInfo BufferInfo::getBufferInfo() const {
	VkBufferCreateInfo create_info {};

	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.size = bytes;
	create_info.usage = vk_buffer_usage;
	create_info.sharingMode = vk_sharing;

	return create_info;
}

BufferInfo& BufferInfo::size(size_t size) {
	bytes = size;
	return *this;
}

BufferInfo& BufferInfo::usage(VkBufferUsageFlags usage) {
	vk_buffer_usage = usage;
	return *this;
}

/*
 * ImageInfo
 */

VkImageType ImageInfo::getImageType() const {
	if (vk_extent.depth == 1 && vk_extent.height == 1) {
		return VK_IMAGE_TYPE_1D;
	}

	if (vk_extent.depth == 1) {
		return VK_IMAGE_TYPE_2D;
	}

	return VK_IMAGE_TYPE_3D;
}

ImageInfo::ImageInfo(size_t width, size_t height, VkFormat format, VkImageUsageFlags usage)
: AllocationInfo(), vk_format(format), vk_tiling(VK_IMAGE_TILING_OPTIMAL), vk_image_usage(usage), vk_samples(VK_SAMPLE_COUNT_1_BIT) {
	size(width, height);
}

ImageInfo::ImageInfo()
: ImageInfo(0, 0, VK_FORMAT_R8G8B8_UINT, 0) {}

VkImageCreateInfo ImageInfo::getImageInfo() const {
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

ImageInfo& ImageInfo::size(size_t width, size_t height, size_t depth) {
	vk_extent.width = width;
	vk_extent.height = height;
	vk_extent.depth = depth;
	return *this;
}

ImageInfo& ImageInfo::format(VkFormat format) {
	vk_format = format;
	return *this;
}

ImageInfo& ImageInfo::tiling(VkImageTiling tiling) {
	vk_tiling = tiling;
	return *this;
}

ImageInfo& ImageInfo::usage(VkImageUsageFlags usage) {
	vk_image_usage = usage;
	return *this;
}

ImageInfo& ImageInfo::samples(int samples) {
	vk_samples = (VkSampleCountFlagBits) samples;
	return *this;
}

/*
 * Allocator
 */

Allocator::Allocator(Device& device, Instance& instance) {
	VmaVulkanFunctions functions = {};
	functions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	functions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo create_info {};
	create_info.physicalDevice = device.vk_physical_device;
	create_info.device = device.vk_device;
	create_info.pVulkanFunctions = &functions;
	create_info.instance = instance.vk_instance;
	create_info.vulkanApiVersion = VK_API_VERSION_1_0;
	create_info.pAllocationCallbacks = AllocatorCallbackFactory::named("VmaAllocator");

	vmaCreateAllocator(&create_info, &vma_allocator);
}

void Allocator::close() {
	vmaDestroyAllocator(vma_allocator);
}

Buffer Allocator::allocateBuffer(const BufferInfo& info) {
	VkBuffer buffer;
	VmaAllocation allocation;

	const VmaAllocationCreateInfo allocation_info = info.getAllocationInfo();
	const VkBufferCreateInfo buffer_info = info.getBufferInfo();

	if(vmaCreateBuffer(vma_allocator, &buffer_info, &allocation_info, &buffer, &allocation, nullptr) != VK_SUCCESS) {
		throw Exception {"Failed to allocated buffer!"};
	}

	return {buffer, {vma_allocator, allocation}};
}

Image Allocator::allocateImage(const ImageInfo& info) {
	VkImage image;
	VmaAllocation allocation;

	const VmaAllocationCreateInfo allocation_info = info.getAllocationInfo();
	const VkImageCreateInfo image_info = info.getImageInfo();

	if (vmaCreateImage(vma_allocator, &image_info, &allocation_info, &image, &allocation, nullptr) != VK_SUCCESS) {
		throw Exception {"Failed to allocated buffer!"};
	}

	return {image, image_info.format, {vma_allocator, allocation}};
}