
#include "immediate.hpp"
#include "client/vertices.hpp"
#include "buffer/allocator.hpp"
#include "buffer/atlas.hpp"

ImmediateRenderer::ImmediateRenderer(const ResourceManager& assets)
: mode(BillboardMode::TWO_AXIS), assets(assets) {
	this->blank = getSprite("blank");
	this->circle = getSprite("circle");

	// initiate settings to some default values
	setTint(255, 255, 255, 255);
	setFontSize(2);
	setLineSize(2);
	setFacing(1, 0, 0);
	setBillboardMode(BillboardMode::TWO_AXIS);
	setAlignment(VerticalAlignment::TOP);
	setAlignment(HorizontalAlignment::LEFT);
	setFontTilt(0);
}

NinePatch ImmediateRenderer::getNinePatch(const std::string& identifier, int margin) {
	return assets.getAtlas().getNinePatch(identifier, margin);
}

BakedSprite ImmediateRenderer::getSprite(const std::string& identifier) {
	return assets.getAtlas().getBakedSprite(identifier);
}

int ImmediateRenderer::getWidth() const {
	return width * 2; // this is multiplied as width and height contains halved values
}

int ImmediateRenderer::getHeight() const {
	return height * 2; // this is multiplied as width and height contain halved values
}

void ImmediateRenderer::setFontSize(float size) {
	this->font_size = size;
}

void ImmediateRenderer::setFontTilt(float tilt) {
	this->font_tilt = tilt;
}

void ImmediateRenderer::setLineSize(float size) {
	this->line_size = size;
}

void ImmediateRenderer::setTint(Color color) {
	this->color = color;
}

void ImmediateRenderer::setTint(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	setTint({r, g, b, a});
}

void ImmediateRenderer::setFacing(float x, float y, float z) {
	this->target = {x, y, z};
}

void ImmediateRenderer::setFacing(glm::vec3 target) {
	setFacing(target.x, target.y, target.z);
}

void ImmediateRenderer::setFacing(const WorldObject& object) {
	setFacing(object.getPosition());
}

void ImmediateRenderer::setBillboardMode(BillboardMode mode) {
	this->mode = mode;
}

void ImmediateRenderer::setAlignment(VerticalAlignment alignment) {
	this->vertical = alignment;
}

void ImmediateRenderer::setAlignment(HorizontalAlignment alignment) {
	this->horizontal = alignment;
}

void ImmediateRenderer::setAlignment(VerticalAlignment vertical, HorizontalAlignment horizontal) {
	setAlignment(vertical);
	setAlignment(horizontal);
}

/*
 * 2D Primitives
 */

void ImmediateRenderer::drawVertex(float x, float y, float u, float v) {
	mesh_2d.emplace_back(x / width - 1, y / height - 1, u, v, color.r, color.g, color.b, color.a);
}

void ImmediateRenderer::drawVertex(glm::vec2 pos, float u, float v) {
	drawVertex(pos.x, pos.y, u, v);
}

void ImmediateRenderer::drawText(float x, float y, const std::string& text, glm::vec2 extend) {

	float offset = 0;
	glm::vec2 alignment = getTextAlignment(text, extend) * font_size;
	const Font& font = assets.getFont();

	for (char chr : text) {

		Glyph glyph = font.getGlyph(chr);
		BakedSprite sprite = glyph.getSprite();

		float w = glyph.getWidth() * font_size;
		float h = glyph.getHeight() * font_size;

		float sx = x + offset - alignment.x ;
		float ex = sx + w;
		float sy = y - alignment.y;
		float ey = sy + h;

		float tilt = font_tilt * h * -0.25;

		if (glyph.shouldDraw()) {
			drawVertex(sx - tilt, sy, sprite.u1, sprite.v1);
			drawVertex(ex + tilt, ey, sprite.u2, sprite.v2);
			drawVertex(sx + tilt, ey, sprite.u1, sprite.v2);

			drawVertex(sx - tilt, sy, sprite.u1, sprite.v1);
			drawVertex(ex - tilt, sy, sprite.u2, sprite.v1);
			drawVertex(ex + tilt, ey, sprite.u2, sprite.v2);
		}

		offset += w + font_size;

	}

}

void ImmediateRenderer::drawText(glm::vec2 pos, const std::string& text, glm::vec2 extend) {
	drawText(pos.x, pos.y, text, extend);
}

void ImmediateRenderer::drawSprite(float sx, float sy, float w, float h, BakedSprite sprite) {

	const float ex = sx + w;
	const float ey = sy + h;

	drawVertex(sx, sy, sprite.u1, sprite.v1);
	drawVertex(ex, ey, sprite.u2, sprite.v2);
	drawVertex(sx, ey, sprite.u1, sprite.v2);

	drawVertex(sx, sy, sprite.u1, sprite.v1);
	drawVertex(ex, sy, sprite.u2, sprite.v1);
	drawVertex(ex, ey, sprite.u2, sprite.v2);
}

void ImmediateRenderer::drawSprite(glm::vec2 pos, float w, float h, BakedSprite sprite) {
	drawSprite(pos.x, pos.y, w, h, sprite);
}

void ImmediateRenderer::drawCircle(float x, float y, float r) {
	drawSprite(x - r, y - r, 2 * r, 2 * r, this->circle);
}

void ImmediateRenderer::drawCircle(glm::vec2 pos, float r) {
	drawCircle(pos.x, pos.y, r);
}

void ImmediateRenderer::drawLine(glm::vec2 pa, glm::vec2 pb) {

	glm::vec2 ab = pb - pa;
	glm::vec2 pp = glm::normalize(glm::vec2 {-ab.y, ab.x}) * line_size;

	drawVertex(pa + pp, blank.u1, blank.v1);
	drawVertex(pb - pp, blank.u2, blank.v2);
	drawVertex(pb + pp, blank.u1, blank.v2);

	drawVertex(pa + pp, blank.u1, blank.v1);
	drawVertex(pa - pp, blank.u2, blank.v1);
	drawVertex(pb - pp, blank.u2, blank.v2);
}

void ImmediateRenderer::drawLine(float x1, float y1, float x2, float y2) {
	drawLine({x1, y1}, {x2, y2});
}

void ImmediateRenderer::drawTiled(float x, float y, float w, float h, BakedSprite sprite, float sw, float sh) {
	float dw, dh;

	float u1 = sprite.u1;
	float v1 = sprite.v1;

	float sx = (sprite.u2 - sprite.u1) / sw;
	float sy = (sprite.v2 - sprite.v1) / sh;

	for (int i = 0; (dw = w - sw * i) > 0; i ++) {

		float tw = std::min(dw, sw);
		float tx = sw * i;
		float u2 = u1 + tw * sx;

		float x1 = x + tx;
		float x2 = x + tx + tw;
		float x3 = x + tx + tw;
		float x4 = x + tx;

		for (int j = 0; (dh = h - sh * j) > 0; j++) {

			float th = std::min(dh, sh);
			float ty = sh * j;
			float v2 = v1 + th * sy;

			float y1 = y + ty;
			float y2 = y + ty;
			float y3 = y + ty + th;
			float y4 = y + ty + th;

			drawVertex(x1, y1, u1, v1);
			drawVertex(x2, y2, u2, v1);
			drawVertex(x4, y4, u1, v2);

			drawVertex(x2, y2, u2, v1);
			drawVertex(x3, y3, u2, v2);
			drawVertex(x4, y4, u1, v2);
		}
	}
}

void ImmediateRenderer::drawTiled(glm::vec2 pos, float w, float h, BakedSprite sprite, float sw, float sh) {
	drawTiled(pos.x, pos.y, w, h, sprite, sw, sh);
}

void ImmediateRenderer::drawBar(float x, float y, float w, float h, float percentage, BakedSprite sprite, int rows, int columns, int row, float sprite_size) {

	float wbreak = percentage * w;

	if (columns < 3) {
		logger::error("Unable to draw bar atlas with less than 3 columns!");
		return;
	}

	float dx = (sprite.u2 - sprite.u1) / columns;
	float dy = (sprite.v2 - sprite.v1) / rows;

	float v1 = sprite.v1 + dy * row;
	float v2 = sprite.v1 + dy * (row + 1);

	float after_first = sprite.u1 + dx;
	float before_last = sprite.u2 - dx;

	// cap is in column `0` and column `columns - 1`
	BakedSprite left {sprite.u1, v1, after_first, v2};
	BakedSprite center {after_first, v1, before_last, v2};
	BakedSprite right {before_last, v1, sprite.u2, v2};

	float center_length = w - 2 * sprite_size;

	float max_left = std::min(wbreak, sprite_size);                                // starts at `0`, goes to `sprite_size`
	float max_center = std::min(wbreak - sprite_size, center_length);              // starts at `sprite_size` goes to `w - sprite_size`
	float max_right = std::min(wbreak - center_length - sprite_size, sprite_size); // starts at `w - sprite_size` goes to `w`

	// draw parts (is a not below the threshold)
	if (max_left > 0)   drawTiled(x, y, max_left, h, left, sprite_size, sprite_size);
	if (max_center > 0) drawTiled(x + sprite_size, y, max_center, h, center, sprite_size * (columns - 2), sprite_size);
	if (max_right > 0)  drawTiled(x + sprite_size + center_length, y, max_right, h, right, sprite_size, sprite_size);

}

void ImmediateRenderer::drawPatch(float x, float y, float w, float h, float s, const NinePatch& patch, bool fill, bool stroke) {

	float m = s / patch.getMargin();
	float sc = patch.getCenter() * m;
	float sm = patch.getMargin() * m;

	if (fill) {
		drawTiled(x, y, w * s, h * s, patch.getSegment(1, 1), sc, sc);
	}

	if (stroke) {
		// edges
		drawTiled(x - s, y + 0, s, h * s, patch.getSegment(0, 1), sm, sc);
		drawTiled(x + w * s, y + 0, s, h * s, patch.getSegment(2, 1), sm, sc);
		drawTiled(x + 0, y - s, w * s, s, patch.getSegment(1, 0), sc, sm);
		drawTiled(x + 0, y + h * s, w * s, s, patch.getSegment(1, 2), sc, sm);

		// corners
		drawTiled(x - s, y - s, s, s, patch.getSegment(0, 0), sm, sm);
		drawTiled(x + w * s, y - s, s, s, patch.getSegment(2, 0), sm, sm);
		drawTiled(x - s, y + h * s, s, s, patch.getSegment(0, 2), sm, sm);
		drawTiled(x + w * s, y + h * s, s, s, patch.getSegment(2, 2), sm, sm);
	}

}

void ImmediateRenderer::drawPatch(glm::vec2 pos, float w, float h, float s, const NinePatch& patch, bool fill, bool stroke) {
	drawPatch(pos.x, pos.y, w, h, s, patch, fill, stroke);
}

/*
 * 3D Primitives
 */

void ImmediateRenderer::drawBillboardVertex(glm::quat rotation, glm::vec3 offset, float x, float y, float u, float v) {
	drawVertex((rotation * glm::vec3 {-x, -y, 0}) + offset, u, v);
}

glm::quat ImmediateRenderer::getBillboardRotation(glm::vec3 center) const {
	glm::vec3 facing = glm::normalize(target - center);

	const float yaw = atan2(facing.x, facing.z);
	glm::quat ry = glm::angleAxis(yaw, glm::vec3(0, 1, 0));

	if (mode == BillboardMode::TWO_AXIS) {
		const float pitch = atan2(-facing.y, glm::length(glm::vec2(facing.x, facing.z)));
		glm::quat rx = glm::angleAxis(pitch, ry * glm::vec3(1, 0, 0));

		return rx * ry;
	}

	return ry;
}

glm::vec3 ImmediateRenderer::getPerpendicular(glm::vec3 normal, float angle) const {
	glm::vec3 axis = glm::cross(normal, glm::vec3(1.0f, 0.0f, 0.0f));

	if (glm::length(axis) < 0.01) {
		axis = glm::cross(normal, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, normal);
	return glm::normalize(glm::vec3(rotation * glm::vec4(axis, 0.0f)));
}

glm::vec2 ImmediateRenderer::getTextAlignment(const std::string& text, glm::vec2 extend) const {

	float mx = static_cast<int>(horizontal) / 2.0f;
	float my = static_cast<int>(vertical) / 2.0f;

	glm::vec2 offset = {
		extend.x < 0 ? 0 : extend.x * mx / font_size,
		extend.y < 0 ? 0 : extend.y * my / font_size
	};

	const Font& font = assets.getFont();

	float ox = 0;
	float oy = font.getSize();

	// special case as then we don't need to iterate the string
	if (horizontal == HorizontalAlignment::LEFT) {
		return glm::vec2 {0, oy * my} - offset;
	}

	for (char chr : text) {
		ox += font.getGlyph(chr).getWidth() + 1;
	}

	return glm::vec2 {ox * mx, oy * my} - offset;

}

void ImmediateRenderer::drawVertex(float x, float y, float z, float u, float v) {
	drawVertex({x, y, z}, u, v);
}

void ImmediateRenderer::drawVertex(glm::vec3 pos, float u, float v) {
	mesh_3d.emplace_back(pos.x, pos.y, pos.z, u, v, color.r, color.g, color.b, color.a);
}

void ImmediateRenderer::drawText(float x, float y, float z, const std::string& text, glm::vec2 extend) {
	drawText({x, y, z}, text, extend);
}

void ImmediateRenderer::drawText(glm::vec3 pos, const std::string& text, glm::vec2 extend) {

	float offset = 0;
	glm::quat rot = getBillboardRotation(pos);

	glm::vec2 alignment = getTextAlignment(text, extend) * font_size;
	const Font& font = assets.getFont();

	for (char chr : text) {

		Glyph glyph = font.getGlyph(chr);
		BakedSprite sprite = glyph.getSprite();

		float w = glyph.getWidth() * font_size;
		float h = glyph.getHeight() * font_size;

		float sx = offset - alignment.x;
		float ex = sx + w;
		float sy = - alignment.y;
		float ey = sy + h;

		float tilt = font_tilt * h * -0.25;

		if (glyph.shouldDraw()) {
			drawBillboardVertex(rot, pos, sx - tilt, sy, sprite.u1, sprite.v1);
			drawBillboardVertex(rot, pos, ex + tilt, ey, sprite.u2, sprite.v2);
			drawBillboardVertex(rot, pos, sx + tilt, ey, sprite.u1, sprite.v2);

			drawBillboardVertex(rot, pos, sx - tilt, sy, sprite.u1, sprite.v1);
			drawBillboardVertex(rot, pos, ex - tilt, sy, sprite.u2, sprite.v1);
			drawBillboardVertex(rot, pos, ex + tilt, ey, sprite.u2, sprite.v2);
		}

		offset += w + font_size;

	}

}

void ImmediateRenderer::drawSprite(float x, float y, float z, float w, float h, BakedSprite sprite) {
	drawSprite({x, y, z}, w, h, sprite);
}

void ImmediateRenderer::drawSprite(glm::vec3 pos, float w, float h, BakedSprite sprite) {

	const float sx = -w / 2;
	const float sy = -h / 2;
	const float ex = sx + w;
	const float ey = sy + h;

	glm::quat rot = getBillboardRotation(pos);

	drawBillboardVertex(rot, pos, sx, sy, sprite.u1, sprite.v1);
	drawBillboardVertex(rot, pos, ex, ey, sprite.u2, sprite.v2);
	drawBillboardVertex(rot, pos, sx, ey, sprite.u1, sprite.v2);

	drawBillboardVertex(rot, pos, sx, sy, sprite.u1, sprite.v1);
	drawBillboardVertex(rot, pos, ex, sy, sprite.u2, sprite.v1);
	drawBillboardVertex(rot, pos, ex, ey, sprite.u2, sprite.v2);

}

void ImmediateRenderer::drawCircle(float x, float y, float z, float r) {
	drawCircle({x, y, z}, r);
}

void ImmediateRenderer::drawCircle(glm::vec3 pos, float r) {
	drawSprite(pos, 2 * r, 2 * r, this->circle);
}

void ImmediateRenderer::drawLine(float x1, float y1, float z1, float x2, float y2, float z2) {
	drawLine({x1, y1, z1}, {x2, y2, z2});
}

void ImmediateRenderer::drawLine(glm::vec3 pa, glm::vec3 pb) {
	glm::vec3 ab = glm::normalize(pb - pa);

	float radius = line_size / 2;
	int sides = 4;
	float slice = (2 * glm::pi<float>()) / sides;

	for (int i = 0; i < sides; i ++) {
		float ac = (i + 0) * slice + glm::pi<float>() / 4;
		float an = (i + 1) * slice + glm::pi<float>() / 4;

		glm::vec3 vc = getPerpendicular(ab, ac) * radius;
		glm::vec3 vn = getPerpendicular(ab, an) * radius;

		//               / Pa + Vc           / Pb + Vc
		//        * --- x -- -- -- -- -- -- x
		//      /  Vc /   \ / Pa + Vn        \  / Pb + Vn
		//     *     /     x -- -- -- -- -- -- x
		//    |     * _.-`  |                  |
		//    |   Pa    Vn  |                  |
		//     *           * -- -- -- -- -- -- *
		//      \         /                   /
		//        * --- * -- -- -- -- -- -- *

		// side
		drawVertex(pa + vc, blank.u1, blank.v1);
		drawVertex(pb + vn, blank.u2, blank.v2);
		drawVertex(pb + vc, blank.u1, blank.v2);

		drawVertex(pa + vc, blank.u1, blank.v1);
		drawVertex(pa + vn, blank.u2, blank.v1);
		drawVertex(pb + vn, blank.u2, blank.v2);

		// caps
		drawVertex(pa     , blank.u1, blank.v1);
		drawVertex(pa + vn, blank.u2, blank.v1);
		drawVertex(pa + vc, blank.u2, blank.v2);

		drawVertex(pb     , blank.u1, blank.v1);
		drawVertex(pb + vn, blank.u2, blank.v1);
		drawVertex(pb + vc, blank.u2, blank.v2);
	}

}

void ImmediateRenderer::drawTiled(float x, float y, float z, float w, float h, BakedSprite sprite, float sw, float sh) {
	drawTiled({x, y, z}, w, h, sprite, sw, sh);
}

void ImmediateRenderer::drawTiled(glm::vec3 pos, float w, float h, BakedSprite sprite, float sw, float sh) {
	float dw, dh;

	float u1 = sprite.u1;
	float v1 = sprite.v1;

	float sx = (sprite.u2 - sprite.u1) / sw;
	float sy = (sprite.v2 - sprite.v1) / sh;

	float cx = w / 2;
	float cy = h / 2;

	glm::quat rot = getBillboardRotation(pos);

	for (int i = 0; (dw = w - sw * i) > 0; i ++) {

		float tw = std::min(dw, sw);
		float tx = sw * i;
		float u2 = u1 + tw * sx;

		float x1 = tx - cx;
		float x2 = tx + tw - cx;
		float x3 = tx + tw - cx;
		float x4 = tx - cx;

		for (int j = 0; (dh = h - sh * j) > 0; j++) {

			float th = std::min(dh, sh);
			float ty = sh * j;
			float v2 = v1 + th * sy;

			float y1 = ty - cy;
			float y2 = ty - cy;
			float y3 = ty + th - cy;
			float y4 = ty + th - cy;

			drawBillboardVertex(rot, pos, x1, y1, u1, v1);
			drawBillboardVertex(rot, pos, x2, y2, u2, v1);
			drawBillboardVertex(rot, pos, x4, y4, u1, v2);

			drawBillboardVertex(rot, pos, x2, y2, u2, v1);
			drawBillboardVertex(rot, pos, x3, y3, u2, v2);
			drawBillboardVertex(rot, pos, x4, y4, u1, v2);
		}
	}
}

void ImmediateRenderer::prepare(VkExtent2D extent) {

	this->width = extent.width / 2;
	this->height = extent.height / 2;
	mesh_2d.clear();
	mesh_3d.clear();

}

void ImmediateRenderer::write(RenderSystem& system, BasicBuffer& buffer_3d, BasicBuffer& buffer_2d) {
	buffer_3d.checkedWrite(system, mesh_3d.data(), mesh_3d.size());
	buffer_2d.checkedWrite(system, mesh_2d.data(), mesh_2d.size());
}