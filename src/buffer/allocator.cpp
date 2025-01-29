
#include "allocator.hpp"

std::unordered_map<VkBuffer, std::string> buffers {};
std::unordered_map<VkImage, std::string> images {};

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

ImageInfo::ImageInfo(int width, int height, VkFormat format, VkImageUsageFlags usage)
: AllocationInfo(), vk_format(format), vk_image_usage(usage), vk_samples(VK_SAMPLE_COUNT_1_BIT), vk_layers(1), vk_levels(1) {
	size(width, height);
}

ImageInfo::ImageInfo()
: ImageInfo(0, 0, VK_FORMAT_R8G8B8A8_UINT, 0) {}

VkImageCreateInfo ImageInfo::getImageInfo() const {
	VkImageCreateInfo create_info {};

	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.extent = vk_extent;
	create_info.mipLevels = vk_levels;
	create_info.arrayLayers = vk_layers;
	create_info.format = vk_format;
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.usage = vk_image_usage;
	create_info.sharingMode = vk_sharing;
	create_info.samples = vk_samples;
	create_info.flags = 0;

	return create_info;
}

ImageInfo& ImageInfo::size(int width, int height) {
	vk_extent.width = width;
	vk_extent.height = height;
	vk_extent.depth = 1;
	return *this;
}

ImageInfo& ImageInfo::format(VkFormat format) {
	vk_format = format;
	return *this;
}

ImageInfo& ImageInfo::usage(VkImageUsageFlags usage) {
	vk_image_usage = usage;
	return *this;
}

ImageInfo& ImageInfo::layers(int layers) {
	vk_layers = layers;
	return *this;
}

ImageInfo& ImageInfo::levels(int levels) {
	vk_levels = levels;
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

	if (vmaCreateBuffer(vma_allocator, &buffer_info, &allocation_info, &buffer, &allocation, nullptr) != VK_SUCCESS) {
		throw Exception {"Failed to allocate buffer!"};
	}

	buffers[buffer] = "Unnamed Buffer";

	return {buffer, {vma_allocator, allocation}};
}

Image Allocator::allocateImage(const ImageInfo& info) {
	VkImage image;
	VmaAllocation allocation;

	const VmaAllocationCreateInfo allocation_info = info.getAllocationInfo();
	const VkImageCreateInfo image_info = info.getImageInfo();

	if (vmaCreateImage(vma_allocator, &image_info, &allocation_info, &image, &allocation, nullptr) != VK_SUCCESS) {
		throw Exception {"Failed to allocate image!"};
	}

	images[image] = "Unnamed Image";

	return {image, image_info.format, {vma_allocator, allocation}};
}