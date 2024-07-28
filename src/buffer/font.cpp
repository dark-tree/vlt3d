
#include "font.hpp"
#include "atlas.hpp"
#include "util/exception.hpp"

/*
 * Glyph
 */

Glyph::Glyph(UnbakedSprite unbaked, ImageData image, int height, bool draw)
: Glyph(unbaked.w, height, unbaked.bake(image.width(), image.height()), draw) {}

Glyph::Glyph(int width, int height, BakedSprite sprite, bool draw)
: width(width), height(height), baked(sprite), draw(draw) {}

int Glyph::getWidth() const {
	return width;
}

int Glyph::getHeight() const {
	return height;
}

BakedSprite Glyph::getSprite() const {
	return baked;
}

bool Glyph::shouldDraw() const {
	return draw;
}

/*
 * Font Overrides
 */

Font::Overrides::Overrides(const TextTreeList* overrides) {

	if (!overrides) {
		return;
	}

	for (auto& entry : *overrides) {
		auto override = entry->as<TextTreeDict>();

		int codepoint = *override->get<TextTreeInt>("codepoint");
		int start = *override->get<TextTreeInt>("start");
		int width = *override->get<TextTreeInt>("width");

		if (cache.contains(codepoint)) {
			throw Exception {"Duplicate codepoint override in font definition"};
		}

		cache[codepoint] = {start, width};
	}

}

bool Font::Overrides::has(int codepoint) const {
	return cache.contains(codepoint);
}

Font::Override Font::Overrides::get(int codepoint) const {
	return cache.at(codepoint);
}

/*
 * Font
 */

UnbakedSprite Font::scanBlock(int code, int bx, int by, int ix, int iy, ImageData image, Overrides& overrides) {
	int min = size;
	int max = 0;

	int ox = bx * size;
	int oy = by * size;
	int px = ox + ix;
	int py = oy + iy;

	if (overrides.has(code)) {
		Override override = overrides.get(code);
		return {override.start, oy, override.width, size};
	}

	if (monospaced) {
		return {ox, oy, size, size};
	}

	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
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

Font::Font(bool monospaced, int size)
: monospaced(monospaced), size(size) {}

void Font::addCodePage(Atlas& atlas, const std::string& identifier, int base, Overrides& overrides) {
	ImageData image = atlas.getImage();
	UnbakedSprite page = atlas.getUnbakedSprite(identifier);

	if (page.w % size != 0 || page.h % size != 0) {
		auto dim = std::to_string(size);
		throw Exception {"Code page " + identifier + " is not divisible into " + dim + "x" + dim + " blocks"};
	}

	int line = page.w / size;
	for (int x = 0; x < page.w / size; x ++) {
		for (int y = 0; y < page.h / size; y ++) {
			int code = x + y * line + base;
			glyphs.try_emplace(code, page.combine(scanBlock(code, x, y, page.x, page.y, image, overrides)), image, size, true /* TODO */);
		}
	}
}

void Font::addFallback(Atlas& atlas) {
	this->fallback = {size, size, atlas.getBakedSprite("font-fallback"), true};
}

Glyph Font::getGlyph(int codepoint) const {
	try {
		return glyphs.at(codepoint);
	} catch (std::out_of_range& first) {
		return fallback;
	}
}

int Font::getSize() const {
	return size;
}

Font Font::loadFromFile(Atlas& atlas, const std::string& filename) {

	TextTree::Input input {filename};
	auto root = input.root()->as<TextTreeDict>();

	bool monospace = *root->get<TextTreeBool>("monospace");
	int sizing = *root->get<TextTreeInt>("sizing");
	auto exceptions = root->getNullable<TextTreeList>("overrides");

	Font font {monospace, sizing};
	Overrides overrides {exceptions};

	for (auto& entry : *root->get<TextTreeList>("pages")) {
		auto page = entry->as<TextTreeDict>();

		int base = *page->get<TextTreeInt>("base");
		std::string path = page->get<TextTreeString>("sprite")->copy();

		font.addCodePage(atlas, path, base, overrides);
	}

	font.addFallback(atlas);
	return font;
}