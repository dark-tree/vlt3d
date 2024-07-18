#pragma once

#include "external.hpp"
#include "image.hpp"

struct BakedSprite {

	public:

		READONLY float u1, v1, u2, v2;

		BakedSprite() = default;
		BakedSprite(float u1, float v1, float u2, float v2);

		BakedSprite grid(int rows, int columns, int row, int column) const;
		BakedSprite column(int length, int index) const;
		BakedSprite row(int length, int index) const;

	public:

		static BakedSprite identity();

};

class UnbakedSprite {

	public:

		READONLY int x, y, w, h;

		UnbakedSprite() = default;
		UnbakedSprite(int x, int y, int w, int h);

	public:

		bool intersects(int x1, int y1, int w, int h) const;
		UnbakedSprite combine(UnbakedSprite other) const;
		BakedSprite bake(const ImageData& image) const;
		BakedSprite bake(int width, int height) const;

		static UnbakedSprite identity(const ImageData& image);

};

class PairedSprite {

	private:

		READONLY UnbakedSprite unbaked;
		READONLY BakedSprite baked;

	public:

		PairedSprite() = default;
		PairedSprite(UnbakedSprite unbaked, BakedSprite baked);

		UnbakedSprite getUnbaked() const;
		BakedSprite getBaked() const;

		static PairedSprite identity(const ImageData& image);

};
