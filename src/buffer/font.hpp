
#pragma once

#include "external.hpp"
#include "atlas.hpp"
#include "util/exception.hpp"

class Glyph {

	private:

		int width, height;
		BakedSprite baked;

	public:

		Glyph() = default;
		Glyph(UnbakedSprite unbaked, ImageData image, int height)
		: width(unbaked.w), height(height), baked(unbaked.bake(image.width(), image.height())) {}

	public:

		int getWidth() const {
			return width;
		}

		int getHeight() const {
			return height;
		}

		BakedSprite getSprite() const {
			return baked;
		}

};

class Font {

	private:

		std::unordered_map<int, Glyph> glyphs;

		UnbakedSprite scanBlock(int bx, int by, int ix, int iy, int size, ImageData image) {
			int min = size;
			int max = 0;

			int ox = bx * size;
			int oy = by * size;
			int px = ox + ix;
			int py = oy + iy;

			for (int x = 0; x < size; x ++) {
				for (int y = 0; y < size; y ++) {
					uint8_t a = image.pixel(x + px, y + py)[3];

					if (a > 250) {
						if (x > max) max = x;
						if (x < min) min = x;
						break;
					}
				}
			}

			// there are no pixels set in this block
			if (min > max) {
				return {ox, oy, size, size};
			}

			return {ox + min, oy, max - min + 1, size};
		}

	public:

		void addCodePage(Atlas& atlas, const std::string& identifier, int size, int base) {
			ImageData image = atlas.getImage();
			UnbakedSprite page = atlas.getUnbakedSprite(identifier);

			if (page.w % size != 0 || page.h % size != 0) {
				auto dim = std::to_string(size);
				throw Exception {"Code page " + identifier + " is not divisible into " + dim + "x" + dim + " blocks"};
			}

			int line = page.w / size;
			for (int x = 0; x < page.w / size; x ++) {
				for (int y = 0; y < page.h / size; y ++) {
					glyphs.try_emplace(x + y * line + base, page.combine(scanBlock(x, y, page.x, page.y, size, image)), image, size);
				}
			}
		}

		Glyph getGlyph(int unicode) {
			return glyphs[unicode];
		}

};