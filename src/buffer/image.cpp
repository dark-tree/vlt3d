
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

ImageData ImageData::scaled(ImageScaling scaling) {

	int sw = std::max(1, w / 2);
	int sh = std::max(1, h / 2);

	ImageData scaled = ImageData::allocate(sw, sh, channels());

	if (scaling == ImageScaling::IGNORE_CONTENT) {
		return scaled;
	}

	if (scaling == ImageScaling::NEAREST) {
		for (int y = 0; y < sh; y ++) {
			for (int x = 0; x < sw; x ++) {
				memcpy(scaled.pixel(x, y), pixel(x * 2, y * 2), channels());
			}
		}

		return scaled;
	}

	UNREACHABLE;

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

void ImageData::clear(std::initializer_list<uint8_t> value) {
	if ((int) value.size() != channels()) {
		throw Exception {"Can't clear image with given value, invalid channel count!"};
	}

	for (int y = 0; y < (int) h; y ++) {
		for (int x = 0; x < (int) w; x++) {
			memcpy(pixel(x, y), value.begin(), c);
		}
	}
}

void ImageData::save(const std::string& path) const {
	if (!stbi_write_png(path.c_str(), w, h, c, pixels, w * c)) {
		throw Exception {"Failed to save image '" + path + "'"};
	}
}

ImageData ImageData::loadFromFile(const std::string& path, int channels) {
	int ignored, w, h;
	void* pixels = stbi_load(path.c_str(), &w, &h, &ignored, channels);

	if (!pixels) {
		throw Exception {"Failed to load image from '" + path + "'"};
	}

	return {Type::STB_IMAGE, pixels, w, h, channels};
}

ImageData ImageData::allocate(int w, int h, int channels) {
	return {Type::MALLOCED, calloc(w * h * channels, 1), w, h, channels};
}

ImageData ImageData::view(void* pixels, int w, int h, int channels) {
	return {Type::VIEW, pixels, w, h, channels};
}

Image ImageData::upload(Allocator& allocator, TaskQueue& queue, CommandRecorder& recorder, VkFormat format, bool mipmaps) const {
	ManagedImageDataSet set {*this, mipmaps};
	Image image = set.upload(allocator, queue, recorder, format);

	for (int i = 1; i < set.levels(); i ++) {
		set.level(i).close();
	}

	return image;
}

int ImageData::size() const {
	return width() * height() * channels();
}

int ImageData::width() const {
	return w;
}

int ImageData::height() const {
	return h;
}

int ImageData::channels() const {
	return c;
}

const void* ImageData::data() const {
	return pixels;
}

/*
 * ManagedImageDataSet
 */

void ManagedImageDataSet::resizeImages(int ws, int hs) {
	for (ImageData& image : images) {
		image.resize(ws * image.width(), hs * image.height());
	}
}

ManagedImageDataSet::ManagedImageDataSet(int w, int h, int channels, bool mipmaps)
: ManagedImageDataSet(ImageData::allocate(w, h, channels), mipmaps) {}

ManagedImageDataSet::ManagedImageDataSet(ImageData image, bool mipmaps) {
	images.push_back(image);
	height = image.height();
	layer = 0;

	if (mipmaps) {
		while (image.width() != 1 || image.height() != 1) {
			image = image.scaled(ImageScaling::IGNORE_CONTENT);
			images.push_back(image);
		}
	}
}

int ManagedImageDataSet::levels() const {
	return images.size();
}

int ManagedImageDataSet::layers() const {
	return std::max(1, layer);
}

ImageData ManagedImageDataSet::level(int level) const {
	return images[level];
}

void ManagedImageDataSet::resize(int ws, int hs) {

	// once we have layers you can no longer resize the image vertically
	if ((layer != 0) && (hs != 1)) {
		throw Exception {"Can't resize the image vertically after a layer was already added!"};
	}

	// actually resize all the image levels
	resizeImages(ws, hs);

	// update "base layer" height
	height *= hs;

}

void ManagedImageDataSet::blit(int ox, int oy, ImageData image, ImageScaling scaling) {
	std::vector<ImageData> temporaries;

	for (ImageData layer : images) {

//		// uncomment for mipmap debug mode
//		if (image.width() == 4) image.clear({255, 0, 0, 255});
//		if (image.width() == 2) image.clear({0, 255, 0, 255});
//		if (image.width() == 1) image.clear({0, 0, 255, 255});

		layer.blit(ox, oy, image);

		image = image.scaled(scaling);

		ox /= 2;
		oy /= 2;

		temporaries.push_back(image);
	}

	for (ImageData temporary : temporaries) {
		temporary.close();
	}
}

void ManagedImageDataSet::addLayer(ImageData image, ImageScaling scaling) {

	// offset to the next empty spot
	const int offset = layer * height;

	// width never changes when adding layers, the image only gets longer
	if (image.width() != level(0).width() || image.height() != height) {
		throw Exception {"Image dimensions don't match the sprite array!"};
	}

	// check the "capacity", if too small, double the image in height
	if (level(0).height() <= offset + height) {
		resizeImages(1, 2);
	}

	blit(0, offset, image, scaling);
	layer ++;

}

size_t ManagedImageDataSet::size() const {
	size_t total = 0;

	for (ImageData image : images) {
		total += image.size();
	}

	return total;
}

ImageData ManagedImageDataSet::unified() const {

	int s = 0;
	int w = 0;
	int h = 0;

	for (ImageData image : images) {
		w += image.width();
		h = std::max(h, image.height());
	}

	ImageData collage = ImageData::allocate(w, h);

	for (ImageData image : images) {
		collage.blit(s, 0, image);
		s += image.width();
	}

	return collage;

}

void ManagedImageDataSet::save(const std::string& path) const {
	ImageData image = unified();
	image.save(path);
	image.close();
}

void ManagedImageDataSet::close() {
	for (ImageData image : images) {
		image.close();
	}

	images.clear();
}

Image ManagedImageDataSet::upload(Allocator& allocator, TaskQueue& queue, CommandRecorder& recorder, VkFormat format) const {

	// dimensions of the base layer in the base level
	int layer_width = level(0).width();
	int layer_height = this->height;
	int layer_count = layers();

	// total image size (all layers and levels)
	size_t total = size();
	size_t offset = 0;

	// verify the given format again the first level (all levels are the same)
	if (getFormatInfo(format).size != (size_t) level(0).channels()) {
		throw Exception {"The specified image format doesn't match pixel size!"};
	}

	BufferInfo buffer_builder {total, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
	buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
	buffer_builder.hint(VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

	Buffer staging = allocator.allocateBuffer(buffer_builder);
	std::vector<size_t> offsets;
	offsets.reserve(levels());

	MemoryMap map = staging.access().map();

	// copy images level by level into staging buffer
	for (ImageData image : images) {
		map.write(image.data(), image.size(), offset);
		offsets.push_back(offset);
		offset += image.size();
	}

	// all done now
	map.flush();
	map.unmap();

	ImageInfo image_builder {layer_width, layer_height, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT};
	image_builder.preferred(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	image_builder.hint(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);
	image_builder.layers(layer_count);
	image_builder.levels(levels());

	Image image = allocator.allocateImage(image_builder);

	recorder.transitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, layer_count, levels());

	// transfer the image level by level
	for (int i = 0; i < levels(); i ++) {
		recorder.copyBufferToImage(image, staging, offsets[i], level(i).width(), std::max(1, (height >> i)), layer_count, i);
	}

	recorder.transitionLayout(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layer_count, levels());

	queue.enqueue([staging] () mutable {
		staging.close();
	});

	return image;

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

void Image::setDebugName(const Device& device, const char* name) const {
	VulkanDebug::name(device.vk_device, VK_OBJECT_TYPE_IMAGE, vk_image, name);
}
