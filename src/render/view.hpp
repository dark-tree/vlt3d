#pragma once

#include "external.hpp"
#include "setup/device.hpp"

class ImageSampler {

	public:

		READONLY VkSampler vk_sampler;
		READONLY VkImageView vk_view;

	public:

		ImageSampler() {}

		ImageSampler(VkSampler sampler, VkImageView view)
		: vk_sampler(sampler), vk_view(view) {}

};

class ImageSamplerBuilder {

	private:

		VkImageView vk_view;
		VkSamplerCreateInfo create_info {};

	public:

		ImageSamplerBuilder()
		: vk_view() {}

		ImageSamplerBuilder(VkImageView view)
		: create_info({}), vk_view(view) {
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

			if (vkCreateSampler(device.vk_device, &create_info, nullptr, &sampler) != VK_SUCCESS) {
				throw std::runtime_error("vkCreateSampler: Failed to create image sampler!");
			}

			return {sampler, vk_view};
		}


};

class ImageView {

	public:

		READONLY VkImageView vk_view;
		READONLY VkImage vk_image;

	public:

		ImageView() {}

		ImageView(VkImageView vk_view, VkImage vk_image)
		: vk_view(vk_view), vk_image(vk_image) {}

		ImageSamplerBuilder getSamplerBuilder() {
			return {vk_view};
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

		ImageView build(Device& device) {

			VkImageViewCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.image = image;

			create_info.viewType = type;
			create_info.format = format;
			create_info.components = components;

			create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			create_info.subresourceRange.baseMipLevel = 0;
			create_info.subresourceRange.levelCount = 1;
			create_info.subresourceRange.baseArrayLayer = 0;
			create_info.subresourceRange.layerCount = 1;

			VkImageView view;

			if (vkCreateImageView(device.vk_device, &create_info, nullptr, &view) != VK_SUCCESS) {
				throw std::runtime_error("vkCreateImageView: Failed to create image view!");
			}

			return ImageView {view, image};

		}

};
