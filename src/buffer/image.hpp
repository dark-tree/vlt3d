#pragma once

#include "external.hpp"
#include "render/view.hpp"

class ImageFile {

	private:

		READONLY void* pixels;
		READONLY int w, h, c;

	public:

		ImageFile(const std::string& path, int channels = 4) {
			int ignored;
			this->c = channels;

			pixels = stbi_load(path.c_str(), &w, &h, &ignored, channels);

			if (!pixels) {
				throw std::runtime_error("stbi_load: Failed to load texture from '" + path + "'");
			}
		}

		void close() {
			if (pixels != nullptr) stbi_image_free(pixels);
		}

	public:

		inline size_t size() const {
			return width() * height() * channels();
		}

		inline size_t width() const {
			return w;
		}

		inline size_t height() const {
			return h;
		}

		inline size_t channels() const {
			return c;
		}

		inline const void* data() const {
			return pixels;
		}

};

class Image {

	public:

		READONLY VkImage vk_image;
		READONLY VkFormat vk_format;
		READONLY MemoryAccess memory;

	public:

		Image() {}

		Image(VkImage vk_image, VkFormat vk_format)
		: vk_image(vk_image), vk_format(vk_format), memory() {}

		Image(VkImage vk_image, VkFormat vk_format, MemoryAccess memory)
		: vk_image(vk_image), vk_format(vk_format), memory(memory) {}

		ImageViewBuilder getViewBuilder() {
			return ImageViewBuilder {vk_image, vk_format};
		}

};
