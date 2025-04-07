#pragma once

#include "external.hpp"
#include "buffer/buffer.hpp"
#include "buffer/image.hpp"
#include "setup/instance.hpp"

extern std::unordered_map<VkBuffer, std::string> buffers;
extern std::unordered_map<VkImage, std::string> images;

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

		/**
		 * Memory properties that MUST be met
		 */
		inline S& required(VkMemoryPropertyFlags flags) {
			vk_required_flags = flags;
			return (S&) *this;
		}

		/**
		 * Memory properties that are preferred but not required
		 */
		inline S& preferred(VkMemoryPropertyFlags flags) {
			vk_preferred_flags = flags;
			return (S&) *this;
		}

		/**
		 * Allows setting VMA flags for this allocation
		 */
		inline S& flags(VmaAllocationCreateFlagBits flags) {
			vma_flags = flags;
			return (S&) *this;
		}

		/**
		 * Sets some VMA usage hints for this allocation
		 */
		inline S& hint(VmaMemoryUsage usage) {
			vma_usage = usage;
			return (S&) *this;
		}

		/**
		 * Can be called to mark the allocated object as "Concurrent" that is,
		 * that is can be used from multiple queue families the the same time
		 */
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

		BufferInfo();
		BufferInfo(size_t size, VkBufferUsageFlags usage);

		VkBufferCreateInfo getBufferInfo() const;

	public:

		/**
		 * Sets a size (in bytes) of the buffer
		 */
		BufferInfo& size(size_t size);

		/**
		 * Sets a bitfield of flags that specifies the valid usages
		 * of the created buffers, some common values include:
		 * VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
		 */
		BufferInfo& usage(VkBufferUsageFlags usage);

};

class ImageInfo : public AllocationInfo<ImageInfo> {

	private:

		READONLY VkExtent3D vk_extent;
		READONLY VkFormat vk_format;
		READONLY VkImageUsageFlags vk_image_usage;
		READONLY VkSampleCountFlagBits vk_samples;
		READONLY int vk_layers;
		READONLY int vk_levels;

	public:

		ImageInfo();
		ImageInfo(int width, int height, VkFormat format, VkImageUsageFlags usage);

		VkImageCreateInfo getImageInfo() const;

	public:

		/**
		 * Sets the dimensions (in pixels) of the image, as images
		 * can also be 3D there is also an optional depth parameter
		 */
		ImageInfo& size(int width, int height);

		/**
		 * Sets how the individual pixels will be stored in memory
		 * and the number of available channels
		 */
		ImageInfo& format(VkFormat format);

		/**
		 * Sets a bitfield of flags that specifies the valid usages
		 * of the created images, some common values include:
		 * VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
		 */
		ImageInfo& usage(VkImageUsageFlags usage);

		/**
		 * The number of layers in the image
		 * where each layers has the given dimensions
		 */
		 ImageInfo& layers(int layers);

		/**
		 * The number of minification (mip)
		 * levels in the image
		 */
		ImageInfo& levels(int levels);

		/**
		 * The number of samples to use in a multisampling
		 */
		ImageInfo& samples(int samples);

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
		Allocator(Device& device, Instance& instance);

		void close();

	public:

		/**
		 * Allocates a new Vulkan Buffer with the specified memory properties
		 */
		Buffer allocateBuffer(const BufferInfo& info);

		/**
		 * Allocates a new Vulkan Image with the specified memory properties
		 */
		Image allocateImage(const ImageInfo& info);

};