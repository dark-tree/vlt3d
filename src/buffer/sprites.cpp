
#include "sprites.hpp"

/*
 * BakedSprite
 */

BakedSprite::BakedSprite(float u1, float v1, float u2, float v2)
: u1(u1), v1(v1), u2(u2), v2(v2) {}

BakedSprite BakedSprite::identity() {
	return {0, 0, 1, 1};
}


/*
 * UnbakedSprite
 */

UnbakedSprite::UnbakedSprite(int x, int y, int w, int h)
: x(x), y(y), w(w), h(h) {}

bool UnbakedSprite::intersects(int x1, int y1, int w, int h) const {
	if (x >= x1 + w || x1 >= x + this->w) return false;
	if (y >= y1 + h || y1 >= y + this->h) return false;

	return true;
}

UnbakedSprite UnbakedSprite::combine(UnbakedSprite other) const {
	return {x + other.x, y + other.y, other.w, other.h};
}

BakedSprite UnbakedSprite::bake(const ImageData& image) const {
	return bake(image.width(), image.height());
}

BakedSprite UnbakedSprite::bake(int width, int height) const {
	const int x1 = x + 0;
	const int y1 = y + 0;
	const int x2 = x + w;
	const int y2 = y + h;

	const float margin = 0.001f;

	return {
	(x1 + margin) / (float) width,
	(y1 + margin) / (float) height,
	(x2 - margin) / (float) width,
	(y2 - margin) / (float) height
	};
}

UnbakedSprite UnbakedSprite::identity(const ImageData& image) {
	return {0, 0, (int) image.width(), (int) image.height()};
}

/*
 * PairedSprite
 */

PairedSprite::PairedSprite(UnbakedSprite unbaked, BakedSprite baked)
: unbaked(unbaked), baked(baked) {}

UnbakedSprite PairedSprite::getUnbaked() const {
	return unbaked;
}

BakedSprite PairedSprite::getBaked() const {
	return baked;
}

PairedSprite PairedSprite::identity(const ImageData& image) {
	return {UnbakedSprite::identity(image), BakedSprite::identity()};
}