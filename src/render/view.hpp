#pragma once

#include "external.hpp"
#include "setup/device.hpp"
#include "setup/callback.hpp"
#include "setup/debug.hpp"

class ImageSampler {

	public:

		READONLY VkSampler vk_sampler;
		READONLY VkImageView vk_view;

	public:

		ImageSampler() = default;
		ImageSampler(VkSampler sampler, VkImageView view)
		: vk_sampler(sampler), vk_view(view) {}

		void close(Device& device) {
			vkDestroySampler(device.vk_device, vk_sampler, AllocatorCallbackFactory::named("Sampler"));
		}

		void setDebugName(const Device& device, const char* name) const {
			VulkanDebug::name(device.vk_device, VK_OBJECT_TYPE_SAMPLER, vk_sampler, name);
		}

};

class ImageSamplerBuilder {

	private:

		VkImageView vk_view;
		VkSamplerCreateInfo create_info {};

	public:

		ImageSamplerBuilder() {}
		ImageSamplerBuilder(VkImageView view)
		: vk_view(view), create_info({}) {
			create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			setFilter(VK_FILTER_LINEAR);
			setMode(VK_SAMPLER_ADDRESS_MODE_REPEAT);
			setAnisotropy(0.0f);
			setBorder(VK_BORDER_COLOR_INT_OPAQUE_BLACK);
		}

		ImageSamplerBuilder& setFilter(VkFilter filter) {
			create_info.magFilter = filter;
			create_info.minFilter = filter;
			return *this;
		}

		ImageSamplerBuilder& setMode(VkSamplerAddressMode mode) {
			create_info.addressModeU = mode;
			create_info.addressModeV = mode;
			create_info.addressModeW = mode;
			return *this;
		}

		ImageSamplerBuilder& setAnisotropy(float anisotropy) {
			create_info.anisotropyEnable = (anisotropy > 0);
			create_info.maxAnisotropy = anisotropy;
			return *this;
		}

		ImageSamplerBuilder& setBorder(VkBorderColor border) {
			create_info.borderColor = border;
			return *this;
		}

		ImageSampler build(Device& device) {
			create_info.unnormalizedCoordinates = VK_FALSE;
			create_info.compareEnable = VK_FALSE;
			create_info.compareOp = VK_COMPARE_OP_ALWAYS;

			create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			create_info.mipLodBias = 0.0f;
			create_info.minLod = 0.0f;
			create_info.maxLod = 0.0f;

			VkSampler sampler;

			if (vkCreateSampler(device.vk_device, &create_info, AllocatorCallbackFactory::named("Sampler"), &sampler) != VK_SUCCESS) {
				throw Exception {"Failed to create image sampler!"};
			}

			return {sampler, vk_view};
		}


};

class ImageView {

	public:

		READONLY VkImageView vk_view;

	public:

		ImageView() = default;
		ImageView(VkImageView vk_view)
		: vk_view(vk_view) {}

		ImageSamplerBuilder getSamplerBuilder() {
			return {vk_view};
		}

		void close(Device& device) {
			vkDestroyImageView(device.vk_device, vk_view, AllocatorCallbackFactory::named("View"));
		}

		void setDebugName(const Device& device, const char* name) const {
			VulkanDebug::name(device.vk_device, VK_OBJECT_TYPE_IMAGE_VIEW, vk_view, name);
		}

};

class ImageViewBuilder {

	private:

		VkImage image;
		VkFormat format;

		VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D;
		VkComponentMapping components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};

	public:

		ImageViewBuilder(VkImage image, VkFormat format)
		: image(image), format(format) {}

		void setType(VkImageViewType type) {
			this->type = type;
		}

		void setSwizzle(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a) {
			this->components.r = r;
			this->components.g = g;
			this->components.b = b;
			this->components.a = a;
		}

		ImageView build(Device& device, VkImageAspectFlags aspect) {

			VkImageViewCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.image = image;

			create_info.viewType = type;
			create_info.format = format;
			create_info.components = components;

			create_info.subresourceRange.aspectMask = aspect;
			create_info.subresourceRange.baseMipLevel = 0;
			create_info.subresourceRange.levelCount = 1;
			create_info.subresourceRange.baseArrayLayer = 0;
			create_info.subresourceRange.layerCount = 1;

			VkImageView view;

			if (vkCreateImageView(device.vk_device, &create_info, AllocatorCallbackFactory::named("View"), &view) != VK_SUCCESS) {
				throw Exception ("Failed to create image view!");
			}

			return ImageView {view};

		}

};
