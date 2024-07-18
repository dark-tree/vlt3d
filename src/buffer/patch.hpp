#pragma once

#include "external.hpp"
#include "sprites.hpp"

/**
 * Represents a Nine-Patch sprite, can be rendered using `ImmediateRenderer::drawPatch()`
 * @see https://en.wikipedia.org/wiki/9-slice_scaling
 */
class NinePatch {

	private:

		std::array<BakedSprite, 9> segments;
		int sizes[3];
		int offsets[3];

	public:

		NinePatch(const ImageData& atlas, UnbakedSprite unbaked, int margin, int center);

		BakedSprite getSegment(int xi, int yi) const;
		int getMargin() const;
		int getCenter() const;

};
