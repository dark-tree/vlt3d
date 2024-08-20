
#include "array.hpp"
#include "util/exception.hpp"

int SpriteArray::packImage(ImageData image) {
	int index = sprites.size();
	int offset = index * height;

	if (image.width() != width || image.height() != height) {
		throw Exception {"Image dimensions don't match the sprite array!"};
	}

	if (atlas.height() <= offset + height) {
		atlas.resize(width, atlas.height() * 2);
	}

	atlas.blit(0, offset, image);
	return index;
}

SpriteArray::SpriteArray(int width, int height) {
	this->atlas = ImageData::allocate(width, height);
	this->width = width;
	this->height = height;
}

void SpriteArray::submitFile(const std::string& identifier, const std::string& path) {
	submitImage(identifier, ImageData::loadFromFile(path));
}

void SpriteArray::submitImage(const std::string& identifier, ImageData image) {
	try {
		sprites[identifier] = packImage(image);
	} catch (Exception& exception) {
		throw Exception {"Sprite '" + identifier + "' can't be loaded into the sprite array!", exception};
	}
}

uint32_t SpriteArray::getSpriteIndex(const std::string& identifier) const {
	auto it = sprites.find(identifier);

	if (it == sprites.end()) {
		logger::warn("Using missing sprite for: '" + identifier + "'");
		sprites[identifier] = 0;
		return 0;
	} else {
		return it->second;
	}
}

uint32_t SpriteArray::getSpriteCount() const {
	return sprites.size();
}

const ImageData& SpriteArray::getImage() const {
	return atlas;
}

Image SpriteArray::upload(Allocator& allocator, TaskQueue& queue, CommandRecorder& recorder) const {
	return Image::upload(allocator, queue, recorder, atlas.data(), width, height, atlas.channels(), getSpriteCount(), VK_FORMAT_R8G8B8A8_SRGB);
}

void SpriteArray::close() {
	atlas.close();
	sprites.clear();
}


SpriteArray SpriteArray::createFromDirectory(int width, int height, const std::string& root, ImageData fallback) {
	SpriteArray array {width, height};
	array.submitImage(FALLBACK_KEY, fallback);

	for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
		if (entry.is_regular_file()) {
			const std::filesystem::path& path = entry.path();
			std::string identifier = path.lexically_relative(root).replace_extension().generic_string();
			array.submitFile(identifier, path.generic_string());
		}
	}

	return array;
}
