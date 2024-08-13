
#include "image.hpp"
#include "buffer.hpp"
#include "allocator.hpp"
#include "command/recorder.hpp"
#include "render/view.hpp"

/*
 * ImageData
 */

ImageData::ImageData(Type type, void* pixels, int w, int h, int c)
: type(type), pixels(pixels), w(w), h(h), c(c) {}

void ImageData::resize(int w, int h) {
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

void ImageData::blit(int ox, int oy, ImageData image) {
	if (ox + image.width() > w || oy + image.height() > h) {
		throw Exception {"Can't blit-in the given image, invalid placement!"};
	}

	if (image.channels() != channels()) {
		throw Exception {"Can't blit-in the given image, invalid channel count!"};
	}

	for (int y = 0; y < (int) image.height(); y ++) {
		memcpy(pixel(ox, oy + y), image.pixel(0, y), image.width() * image.channels());
	}
}

uint8_t* ImageData::pixel(int x, int y) {
	return ((uint8_t*) pixels) + (x + y * w) * channels();
}

void ImageData::close() {
	if (pixels != nullptr) {
		if (type == STB_IMAGE) stbi_image_free(pixels);
		if (type == MALLOCED) free(pixels);
	}
}

void ImageData::save(const std::string& path) const {
	stbi_write_png(path.c_str(), w, h, c, pixels, w * c);
}

ImageData ImageData::loadFromFile(const std::string& path, int channels) {
	int ignored, w, h;
	void* pixels = stbi_load(path.c_str(), &w, &h, &ignored, channels);

	if (!pixels) {
		throw Exception {"Failed to load texture from '" + path + "'"};
	}

	return {Type::STB_IMAGE, pixels, w, h, channels};
}

ImageData ImageData::allocate(int w, int h, int channels) {
	return {Type::MALLOCED, malloc(w * h * channels), w, h, channels};
}

Image ImageData::upload(Allocator& allocator, CommandRecorder& recorder, VkFormat format) const {

	BufferInfo buffer_builder {size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
	buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
	buffer_builder.hint(VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

	Buffer staging = allocator.allocateBuffer(buffer_builder);

	MemoryMap map = staging.access().map();
	map.write(data(), size());
	map.flush();
	map.unmap();

	ImageInfo image_builder {w, h, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT};
	image_builder.preferred(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	image_builder.tiling(VK_IMAGE_TILING_OPTIMAL);
	buffer_builder.hint(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	Image image = allocator.allocateImage(image_builder);

	recorder.transitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED);
	recorder.copyBufferToImage(image, staging, w, h);
	recorder.transitionLayout(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	return image;

}

size_t ImageData::size() const {
	return width() * height() * channels();
}

size_t ImageData::width() const {
	return w;
}

size_t ImageData::height() const {
	return h;
}

size_t ImageData::channels() const {
	return c;
}

const void* ImageData::data() const {
	return pixels;
}

/*
 * Image
 */

Image::Image(VkImage vk_image, VkFormat vk_format)
: vk_image(vk_image), vk_format(vk_format), memory() {}

Image::Image(VkImage vk_image, VkFormat vk_format, MemoryAccess memory)
: vk_image(vk_image), vk_format(vk_format), memory(memory) {}

ImageViewBuilder Image::getViewBuilder() const {
	return ImageViewBuilder {vk_image, vk_format};
}

void Image::close() {
	memory.closeImage(vk_image);
}