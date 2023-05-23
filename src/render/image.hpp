#pragma once

#include "external.hpp"
#include "view.hpp"

class Image {

	public:

		READONLY VkImage vk_image;
		READONLY VkFormat vk_format;

	public:

		Image(VkImage vk_image, VkFormat vk_format)
		: vk_image(vk_image), vk_format(vk_format) {}

		ImageViewBuilder builder() {
			return ImageViewBuilder {vk_image, vk_format};
		}

};
