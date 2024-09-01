#pragma once

#include "external.hpp"
#include "memory.hpp"
#include "util/threads.hpp"
#include "setup/callback.hpp"

class Image;
class CommandRecorder;
class Allocator;
class ImageViewBuilder;
class Device;

enum struct ImageScaling {
	IGNORE_CONTENT,
	NEAREST
};

/**
 * Class for representing any CPU-accessible image data
 * both loaded from a file or created dynamically, it implements
 * certain helpful image manipulation methods like `blit()` and `resize()`
 */
class ImageData {

	private:

		enum Type : uint8_t {
			STB_IMAGE,
			MALLOCED,
			VIEW
		};

		READONLY Type type;
		READONLY void* pixels;
		READONLY int w, h, c;

	public:

		ImageData() = default;
		ImageData(Type type, void* pixels, int w, int h, int c);

		/**
		 * Internally reallocates the image, previous data is copied
		 * into the new memory and freed (pointers to old data will become invalid)
		 */
		void resize(int w, int h);

		/**
		 * Returns a scaled-down copy of this image,
		 * useful for mip-map creation
		 */
		ImageData scaled(ImageScaling scaling);

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
		 * Clears the image, setting all pixels in the image to the
		 * given value, must contains EXACTLY 'channels()' components
		 */
		void clear(std::initializer_list<uint8_t> value);

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
		 * Treats the given pointer as the image data
		 * the data is not taken ownership of
		 */
		static ImageData view(void* pixels, int w, int h, int channels = 4);

		/**
		 * Records a copy-to-GPU-memory operation into the
		 * given command buffer, a staging buffer IS used and resulting image returned
		 */
		Image upload(Allocator& allocator, TaskQueue& queue, CommandRecorder& recorder, VkFormat format) const;

	public:

		/// returns the size, in bytes, of the data buffer used by this image
		int size() const;

		/// returns the width, in pixels, of the image
		int width() const;

		/// returns the height, in pixels, of the image
		int height() const;

		/// returns the number of channels (bytes) per pixel
		int channels() const;

		/// returns a pointer to the start of images' data buffer
		const void* data() const;

};

class ManagedImageDataSet {

	private:

		// layer height in pixels
		int height;

		// number of layers
		int layers;

		// mipmaps array
		// TODO std::vector<ImageData>*
		std::vector<ImageData> images;

		// unconditionaly resizes all image levels
		void resizeImages(int ws, int hs);

	public:

		ManagedImageDataSet() = default;
		ManagedImageDataSet(int w, int h, int channels, bool mipmaps);

		int levels() const;
		ImageData level(int level) const;

		/**
		 * Resizes the image by the given multiplier.
		 * Note that this is different than the ImageData::resize method!
		 */
		void resize(int ws, int hs);

		/**
		 * Blit (paste) another, smaller, image into this one
		 * the given image will me scaled down and pasted into all minification layers
		 */
		void blit(int ox, int oy, ImageData image, ImageScaling scaling);

		/**
		 * Adds a new image layer, all layers need to have the same
		 * dimensions. After this method is called the image ca no longer be resized vertically.
		 */
		void addLayer(ImageData image, ImageScaling scaling);

		/**
		 * Get the total combined memory requirement of the whole
		 * image (including all layers and levels) in bytes
		 */
		size_t size() const;

		/**
		 * Allocates a new image that will contain all the
		 * minification levels of this set, can be useful for debugging
		 */
		ImageData unified() const;

		/**
		 * Save the image data as a PNG image under the
		 * given path, can be useful for debugging
		 */
		void save(const std::string& path) const;

		/**
		 * Records a copy-to-GPU-memory operation into the
		 * given command buffer, a staging buffer IS used and resulting image returned
		 */
		Image upload(Allocator& allocator, TaskQueue& queue, CommandRecorder& recorder, VkFormat format) const;

		/**
		 * Free the data held by all the individual
		 * levels of images
		 */
		void close();

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

		void setDebugName(const Device& device, const char* name) const;

	public:

		[[deprecated("Use MangedImageDataSet::upload()")]]
		static Image upload(Allocator& allocator, TaskQueue& queue, CommandRecorder& recorder, const void* pixels, int width, int height, int channels, int layers, VkFormat format);

};
