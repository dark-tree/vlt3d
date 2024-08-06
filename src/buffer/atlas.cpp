
#include "atlas.hpp"
#include "image.hpp"
#include "patch.hpp"
#include "util/util.hpp"
#include "util/logger.hpp"

/*
 * Atlas
 */

Atlas::Atlas(ImageData atlas, const std::unordered_map<std::string, UnbakedSprite>& unbaked, PairedSprite fallback)
: atlas(atlas), fallback(fallback) {
	for (const auto& [key, value] : unbaked) {
		sprites.try_emplace(key, value, value.bake(atlas.width(), atlas.height()));
	}
}

const PairedSprite& Atlas::getSpritePair(const std::string& identifier) const {
	auto it = sprites.find(identifier);

	if (it == sprites.end()) {
		logger::warn("Using missing sprite for: '" + identifier + "'");
		sprites[identifier] = fallback;
		return fallback;
	} else {
		return it->second;
	}
}

BakedSprite Atlas::getBakedSprite(const std::string& identifier) const {
	return getSpritePair(identifier).getBaked();
}

UnbakedSprite Atlas::getUnbakedSprite(const std::string& identifier) const {
	return getSpritePair(identifier).getUnbaked();
}

NinePatch Atlas::getNinePatch(const std::string& identifier, int margin) const {
	const PairedSprite& pair = getSpritePair(identifier);
	const UnbakedSprite& unbaked = pair.getUnbaked();
	int size = std::min(unbaked.w, unbaked.h);

	if (unbaked.w != unbaked.h) {
		logger::warn("Creating nine patch from a non-square sprite '", identifier, "', assumed shorter edge");
	}

	if (size <= margin * 2) {
		logger::error("Nine patch margin is wider than the whole sprite '", identifier, "', margin reduced to fit");
		margin = (size - 1) / 2;
	}

	return {getImage(), unbaked, margin, size - 2 * margin};
}

const ImageData& Atlas::getImage() const {
	return atlas;
}

void Atlas::close() {
	atlas.close();
	sprites.clear();
}

/*
 * AtlasBuilder
 */

bool AtlasBuilder::canPlaceAt(int x, int y, int w, int h) {
	if (x + w > (int) atlas.width()) {
		return false;
	}

	if (y + h > (int) atlas.height()) {
		return false;
	}

	for (const auto& [key, value] : sprites) {
		if (value.intersects(x, y, w, h)) {
			return false;
		}
	}

	return true;
}

UnbakedSprite AtlasBuilder::packUnbakedSprite(ImageData image) {
	for (int x = 0; x < (int) atlas.width(); x ++) {
		for (int y = 0; y < (int) atlas.height(); y ++) {
			int w = image.width();
			int h = image.height();

			if (canPlaceAt(x, y, w, h)) {
				atlas.blit(x, y, image);
				return {x, y, w, h};
			}
		}
	}

	// retry with bigger atlas
	atlas.resize(atlas.width() * 2, atlas.height() * 2);
	return packUnbakedSprite(image);
}

PairedSprite AtlasBuilder::getFallback() const {
	try {
		UnbakedSprite fallback = sprites.at(fallback_key);
		return {fallback, fallback.bake(atlas.width(), atlas.height())};
	} catch (...) {
		return PairedSprite::identity(atlas);
	}
}

AtlasBuilder::AtlasBuilder() {
	atlas = ImageData::allocate(512, 512);
}

void AtlasBuilder::submitFile(const std::string& identifier, const std::string& path) {
	submitImage(identifier, ImageData::loadFromFile(path));
}

void AtlasBuilder::submitImage(const std::string& identifier, ImageData image) {
	sprites[identifier] = packUnbakedSprite(image);
}

void AtlasBuilder::submitFallback(ImageData image) {
	submitImage(fallback_key, image);
}

Atlas AtlasBuilder::build() {
	return {atlas, sprites, getFallback()};
}

Atlas AtlasBuilder::createSimpleAtlas(const std::string& root, ImageData fallback) {
	AtlasBuilder builder;

	for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
		if (entry.is_regular_file()) {
			const std::filesystem::path& path = entry.path();
			std::string identifier = path.lexically_relative(root).replace_extension().generic_string();
			builder.submitFile(identifier, path.generic_string());
		}
	}

	builder.submitFallback(fallback);
	return builder.build();
}

Atlas AtlasBuilder::createIdentityAtlas(ImageData atlas) {
	return {atlas, {}, PairedSprite::identity(atlas)};
}

