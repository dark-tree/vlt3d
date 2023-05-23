#pragma once

#include "external.hpp"
#include "setup/device.hpp"

class ImageView {

	public:

		READONLY VkImageView vk_view;

	public:

		ImageView(VkImageView vk_view)
		: vk_view(vk_view) {}

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

			create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
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

			return ImageView {view};

		}

};
