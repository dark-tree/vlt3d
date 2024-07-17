#pragma once

#include "external.hpp"
#include "sprites.hpp"

class NinePatch {

	public:

//		struct Segment {
//
//			BakedSprite sprite;
//			int width, height;
//			int x, y;
//
//			Segment(BakedSprite sprite, int width, int height, int x, int y);
//
//		};

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
