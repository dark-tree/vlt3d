#pragma once

#include "external.hpp"

class Box2D {

	public:

		READONLY float x1, y1;
		READONLY float x2, y2;

	public:

		Box2D();
		Box2D(float x1, float y1, float x2, float y2);

		float width() const;
		float height() const;
		bool contains(float x, float y) const;
		bool contains(glm::vec2 pos) const;
		bool empty() const;
		glm::vec2 begin() const;
		glm::vec2 end() const;

		Box2D scale(float scalar) const;
		Box2D offset(float x, float y) const;
		Box2D offset(glm::vec2 offset) const;
		Box2D inset(float scalar) const;
		Box2D envelop(Box2D other) const;
		Box2D round() const;
};