
#pragma once

#include "external.hpp"
#include "sprites.hpp"

class ImageData;
class Atlas;

class Glyph {

	private:

		int width, height;
		BakedSprite baked;
		bool draw;

	public:

		Glyph() = default;
		Glyph(UnbakedSprite unbaked, ImageData image, int height, bool draw);

	public:

		/// Returns the width, in pixels, of this glyph
		int getWidth() const;

		/// Returns the height, in pixels, of this glyph
		int getHeight() const;

		/// Returns the scaled sprite that can be directly used to display this glyph
		BakedSprite getSprite() const;

		/// Should this glyph emit any vertex data, should return false for empty sprites (like space, new line etc)
		bool shouldDraw() const;

};

class Font {




	private:

		struct Override {
			int start;
			int width;
		};

		class Overrides {

			private:

				std::unordered_map<int, Override> cache;

			public:

				Overrides(const TextTreeList* overrides);

				/**
				 * Check if the given codepoint has a manual override
				 * MUST be called and checked before calling `get()`
				 */
				bool has(int codepoint) const;

				/**
				 * Get the manual override for a particular codepoint,
				 * MUST be called only if `has()` returned true
				 */
				Override get(int codepoint) const;

		};

	private:

		bool monospaced;
		int size;
		std::unordered_map<int, Glyph> glyphs;

		UnbakedSprite scanBlock(int code, int bx, int by, int ix, int iy, ImageData image, Overrides& overrides);
		void addCodePage(Atlas& atlas, const std::string& identifier, int base, Overrides& overrides);

	private:

		Font(bool monospaced, int size);

	public:

		/**
		 * Get the Glyph for a particular code point
		 * this includes custom characters and normal ones
		 */
		Glyph getGlyph(int unicode) const;

		/**
		 * Get the size of the glyph in the source image
		 * this is also equal to the font height
		 */
		int getSize() const;

	public:

		/**
		 * Loads the font from a .tt definition file
		 */
		static Font loadFromFile(Atlas& atlas, const std::string& filename);

};