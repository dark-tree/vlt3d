#pragma once

#include "external.hpp"
#include "image.hpp"

struct BakedSprite {
	READONLY float u1, v1, u2, v2;

	BakedSprite() {}
	BakedSprite(float u1, float v1, float u2, float v2)
	: u1(u1), v1(v1), u2(u2), v2(v2) {}

	inline static BakedSprite identity() {
		return {0, 0, 1, 1};
	}
};

struct UnbakedSprite {
	READONLY int x, y, w, h;

	UnbakedSprite() {}
	UnbakedSprite(int x, int y, int w, int h)
	: x(x), y(y), w(w), h(h) {}

	bool intersects(int x1, int y1, int w, int h) const {
		if (x >= x1 + w || x1 >= x + this->w) return false;
		if (y >= y1 + h || y1 >= y + this->h) return false;

		return true;
	}

	BakedSprite bake(int width, int height) const {
		const int x1 = x + 0;
		const int y1 = y + 0;
		const int x2 = x + w;
		const int y2 = y + h;

		return {
			(x1 + 0.5f) / (float) width,
			(y2 + 0.5f) / (float) height,
			(x2 + 0.5f) / (float) width,
			(y1 + 0.5f) / (float) height
		};
	}
};

class Atlas {

	private:

		ImageData atlas;
		BakedSprite fallback;
		std::unordered_map<std::string, BakedSprite> sprites;
		friend class AtlasBuilder;

		Atlas(ImageData atlas, const std::unordered_map<std::string, UnbakedSprite>& unbaked, BakedSprite fallback)
		: atlas(atlas), fallback(fallback) {
			for (const auto& [key, value] : unbaked) {
				sprites[key] = value.bake(atlas.width(), atlas.height());
			}
		}

	public:

		BakedSprite getSprite(const std::string& identifier) const {
			return util::fallback_get(sprites, identifier, fallback);
		}

		ImageData& getImage() {
			return atlas;
		}

		void close() {
			atlas.close();
			sprites.clear();
		}

};

class AtlasBuilder {

	private:

		UnbakedSprite* fallback = nullptr;
		ImageData atlas;
		std::unordered_map<std::string, UnbakedSprite> sprites;

		bool canPlaceAt(int x, int y, int w, int h) {
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

		UnbakedSprite packUnbakedSprite(ImageData image) {
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

		BakedSprite getBakedFallback() const {
			if (fallback == nullptr) {
				return BakedSprite::identity();
			}

			return fallback->bake(atlas.width(), atlas.height());
		}

	public:

		AtlasBuilder() {
			atlas = ImageData::allocate(512, 512);
		}

		void submitDirectory(const std::string& identifier) {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(identifier)) {
				if (entry.is_regular_file()) submitFile(entry.path().string());
			}
		}

		void submitFile(const std::string& identifier) {
			submitImage(identifier, ImageData::loadFromFile(identifier));
		}

		void submitImage(const std::string& identifier, ImageData image) {
			sprites[identifier] = packUnbakedSprite(image);
		}

		void submitFallback(const std::string& identifier) {
			fallback = &sprites.at(identifier);
		}

		Atlas build() {
			return {atlas, sprites, getBakedFallback()};
		}

	public:

		static Atlas createSimpleAtlas(const std::string& identifier) {
			AtlasBuilder builder;
			builder.submitDirectory(identifier);
			return builder.build();
		}

		static Atlas createIdentityAtlas(ImageData atlas) {
			return {atlas, {}, BakedSprite::identity()};
		}

};
