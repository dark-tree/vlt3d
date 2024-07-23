
#include "patch.hpp"

//NinePatch::Segment::Segment(BakedSprite sprite, int width, int height, int x, int y)
//: sprite(sprite), width(width), height(height), x(x), y(y) {}

NinePatch::NinePatch(const ImageData& atlas, UnbakedSprite unbaked, int margin, int center)
: sizes(margin, center, margin), offsets(0, margin, center + margin) {
	for (int xi = 0; xi < 3; xi ++) {
		for (int yi = 0; yi < 3; yi ++) {
			segments[xi + yi * 3] = unbaked.combine({offsets[xi], offsets[yi], sizes[xi], sizes[yi]}).bake(atlas);
		}
	}
}

BakedSprite NinePatch::getSegment(int xi, int yi) const {
	return segments[xi + yi * 3];
}

int NinePatch::getMargin() const {
	return sizes[0];
}

int NinePatch::getCenter() const {
	return sizes[1];
}