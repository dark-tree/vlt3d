#pragma once

#include "external.hpp"
#include "render/view.hpp"

class ImageData {

	private:

		enum Type {
			STB_IMAGE,
			MALLOCED,
			UNBACKED
		};

		READONLY Type type;
		READONLY void* pixels;
		READONLY int w, h, c;

	public:

		ImageData() {}

		ImageData(Type type, void* pixels, int w, int h, int c)
		: type(type), pixels(pixels), w(w), h(h), c(c) {}

		void resize(int w, int h) {
			if (w * h * channels() < size()) {
				return;
			}

			ImageData buffer = ImageData::allocate(w, h, channels());
			buffer.blit(0, 0, *this);

			// no matter how our image was allocated, free it now
			close();

			this->pixels = buffer.pixels;
			this->w = w;
			this->h = h;
		}

		void blit(uint32_t ox, uint32_t oy, ImageData image) {
			if (ox + image.width() > w || oy + image.height() > h) {
				throw std::runtime_error("Can't blit-in the given image, invalid placement!");
			}

			if (image.channels() != channels()) {
				throw std::runtime_error("Can't blit-in the given image, invalid channel count!");
			}

			for (int y = 0; y < image.height(); y ++) {
				memcpy(pixel(ox, oy + y), image.pixel(0, y), image.width() * image.channels());
			}
		}

		uint8_t* pixel(int x, int y) {
			return ((uint8_t*) pixels) + (x + y * w) * channels();
		}

		void close() {
			if (pixels != nullptr) {
				if (type == STB_IMAGE) stbi_image_free(pixels);
				if (type == MALLOCED) free(pixels);
			}
		}

		/**
		 * Returns an image data buffer for the image pointer to by the
		 * given file path and of the given number of channels
		 */
		static ImageData loadFromFile(const std::string& path, int channels = 4) {
			int ignored, w, h;
			void* pixels = stbi_load(path.c_str(), &w, &h, &ignored, channels);

			if (!pixels) {
				throw std::runtime_error("stbi_load: Failed to load texture from '" + path + "'");
			}

			return {Type::STB_IMAGE, pixels, w, h, channels};
		}

		/**
		 * Creates a new uninitialized image of the given
		 * dimensions and channel count.
		 */
		static ImageData allocate(int w, int h, int channels = 4) {
			return {Type::MALLOCED, malloc(w * h * channels), w, h, channels};
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
