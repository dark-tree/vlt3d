#pragma once

#include "external.hpp"
#include "memory.hpp"

class Image;
class CommandRecorder;
class Allocator;
class ImageViewBuilder;
class Device;

/**
 * Class for representing any CPU-accessible image data
 * both loaded from a file or created dynamically, it implements
 * certain helpful image manipulation methods like `blit()` and `resize()`
 */
class ImageData {

	private:

		enum Type {
			STB_IMAGE,
			MALLOCED,
			UNBACKED
		};

		READONLY Type type;
		READONLY void* pixels;
		READONLY size_t w, h, c;

	public:

		ImageData() = default;
		ImageData(Type type, void* pixels, int w, int h, int c);

		/**
		 * Internally reallocates the image, previous data is copied
		 * into the new memory and freed (pointers to old data will become invalid!)
		 */
		void resize(int w, int h);

		/**
		 * Blit (paste) another, smaller, image into this one
		 * at the given position, both images MUST share channel counts
		 */
		void blit(int ox, int oy, ImageData image);

		/**
		 * Returns a pointer to the given pixel,
		 * the pointer points to at least `channels()` uint8_t elements
		 */
		uint8_t* pixel(int x, int y);

		/**
		 * Free the data held by this image data
		 * object if that underlying data needs to be freed
		 */
		void close();

		/**
		 * Save the image data as a PNG image under the
		 * given path, can be useful for debugging
		 */
		void save(const std::string& path) const;

		/**
		 * Returns an image data buffer for the image pointer to by the
		 * given file path and of the given number of channels
		 */
		static ImageData loadFromFile(const std::string& path, int channels = 4);

		/**
		 * Creates a new uninitialized image of the given
		 * dimensions and channel count.
		 */
		static ImageData allocate(int w, int h, int channels = 4);

		/**
		 * Records a copy-to-GPU-memory operation into the
		 * given command buffer, a staging buffer IS used and resulting image returned
		 */
		Image upload(Allocator& allocator, CommandRecorder& recorder, VkFormat format) const;

	public:

		/// returns the size, in bytes, of the data buffer used by this image
		size_t size() const;

		/// returns the width, in pixels, of the image
		size_t width() const;

		/// returns the height, in pixels, of the image
		size_t height() const;

		/// returns the number of channels (bytes) per pixel
		size_t channels() const;

		/// returns a pointer to the start of images' data buffer
		const void* data() const;

};

/**
 * Wrapper around the vulkan image class for
 * representing any GPU-accessible image data.
 * It can be created from image data using `upload()`
 */
class Image {

	public:

		READONLY VkImage vk_image;
		READONLY VkFormat vk_format;
		READONLY MemoryAccess memory;

	public:

		Image() = default;
		Image(VkImage vk_image, VkFormat vk_format);
		Image(VkImage vk_image, VkFormat vk_format, MemoryAccess memory);

		ImageViewBuilder getViewBuilder() const;
		void close();

};
