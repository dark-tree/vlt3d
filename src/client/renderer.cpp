
#include "renderer.hpp"
#include "client/vertices.hpp"
#include "buffer/allocator.hpp"
#include "buffer/atlas.hpp"

ImmediateRenderer::ImmediateRenderer(Atlas& atlas, Font& font)
: mode(BillboardMode::TWO_AXIS), atlas(atlas), font(font) {
	this->blank = getSprite("assets/sprites/blank.png");

	// initiate settings to some default values
	setTint(255, 255, 255, 255);
	setFontSize(2);
	setLineSize(2);
	setFacing(1, 0, 0);
	setBillboardMode(BillboardMode::TWO_AXIS);
	setAlignment(VerticalAlignment::TOP);
	setAlignment(HorizontalAlignment::LEFT);
}

BakedSprite ImmediateRenderer::getSprite(const std::string& identifier) {
	return atlas.getBakedSprite(identifier);
}

int ImmediateRenderer::getWidth() const {
	return width;
}

int ImmediateRenderer::getHeight() const {
	return height;
}

void ImmediateRenderer::setFontSize(float size) {
	this->font_size = size;
}

void ImmediateRenderer::setLineSize(float size) {
	this->line_size = size;
}

void ImmediateRenderer::setTint(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
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

/*
 * 2D Primitives
 */

void ImmediateRenderer::drawVertex(float x, float y, float u, float v) {
	mesh_2d.emplace_back(x / width - 1, y / height - 1, u, v, r, g, b, a);
}

void ImmediateRenderer::drawVertex(glm::vec2 pos, float u, float v) {
	drawVertex(pos.x, pos.y, u, v);
}

void ImmediateRenderer::drawText(float x, float y, const std::string& text) {

	float offset = 0;
	glm::vec2 alignment = getTextAlignment(text) * font_size;

	for (char chr : text) {

		Glyph glyph = font.getGlyph(chr);
		BakedSprite sprite = glyph.getSprite();

		float w = glyph.getWidth() * font_size;

		float sx = x + offset - alignment.x;
		float ex = sx + w;
		float sy = y - alignment.y;
		float ey = sy + glyph.getHeight() * font_size;

		drawVertex(sx, sy, sprite.u1, sprite.v1);
		drawVertex(ex, ey, sprite.u2, sprite.v2);
		drawVertex(sx, ey, sprite.u1, sprite.v2);

		drawVertex(sx, sy, sprite.u1, sprite.v1);
		drawVertex(ex, sy, sprite.u2, sprite.v1);
		drawVertex(ex, ey, sprite.u2, sprite.v2);

		offset += w + font_size;

	}

}

void ImmediateRenderer::drawText(glm::vec2 pos, const std::string& text) {
	drawText(pos.x, pos.y, text);
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

/*
 * 3D Primitives
 */

void ImmediateRenderer::drawBillboardVertex(glm::quat rotation, glm::vec3 offset, float x, float y, float u, float v) {
	drawVertex((rotation * glm::vec3 {x, y, 0}) + offset, u, v);
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

glm::vec2 ImmediateRenderer::getTextAlignment(const std::string& text) const {

	float mx = static_cast<int>(horizontal) / 2.0f;
	float my = static_cast<int>(vertical) / 2.0f;

	float ox = 0;
	float oy = font.getSize();

	// special case as then we don't need to iterate the string
	if (horizontal == HorizontalAlignment::LEFT) {
		return {0, oy * my};
	}

	for (char chr : text) {
		ox += font.getGlyph(chr).getWidth() + 1;
	}

	return {ox * mx, oy * my};

}

void ImmediateRenderer::drawVertex(float x, float y, float z, float u, float v) {
	drawVertex({x, y, z}, u, v);
}

void ImmediateRenderer::drawVertex(glm::vec3 pos, float u, float v) {
	mesh_3d.emplace_back(pos.x, pos.y, pos.z, u, v, r, g, b, a);
}

void ImmediateRenderer::drawText(float x, float y, float z, const std::string& text) {
	drawText({x, y, z}, text);
}

void ImmediateRenderer::drawText(glm::vec3 pos, const std::string& text) {

	float offset = 0;
	glm::quat rot = getBillboardRotation(pos);

	glm::vec2 alignment = getTextAlignment(text) * font_size;

	for (char chr : text) {

		Glyph glyph = font.getGlyph(chr);
		BakedSprite sprite = glyph.getSprite();

		float w = glyph.getWidth() * font_size;
		float h = glyph.getHeight() * font_size;

		float sx = offset - alignment.x;
		float ex = sx + w;
		float sy = - alignment.y;
		float ey = sy + h;

		drawBillboardVertex(rot, pos, sx, sy, sprite.u1, sprite.v1);
		drawBillboardVertex(rot, pos, ex, ey, sprite.u2, sprite.v2);
		drawBillboardVertex(rot, pos, sx, ey, sprite.u1, sprite.v2);

		drawBillboardVertex(rot, pos, sx, sy, sprite.u1, sprite.v1);
		drawBillboardVertex(rot, pos, ex, sy, sprite.u2, sprite.v1);
		drawBillboardVertex(rot, pos, ex, ey, sprite.u2, sprite.v2);

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

void ImmediateRenderer::drawLine(float x1, float y1, float z1, float x2, float y2, float z2) {
	drawLine({x1, y1, z1}, {x2, y2, z2});
}

void ImmediateRenderer::drawLine(glm::vec3 pa, glm::vec3 pb) {
	glm::vec3 ab = glm::normalize(pb - pa);

	float radius = line_size / 2;
	int sides = 6;
	float slice = (2 * glm::pi<float>()) / sides;

	for (int i = 0; i < sides; i ++) {
		float ac = (i + 0) * slice;
		float an = (i + 1) * slice;

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

void ImmediateRenderer::prepare(VkExtent2D extent) {

	this->width = extent.width / 2;
	this->height = extent.height / 2;
	mesh_2d.clear();
	mesh_3d.clear();

}

void ImmediateRenderer::getBuffers(Allocator& allocator, Buffer* buf_3d, int* len_3d, Buffer* buf_2d, int* len_2d) {

	size_t alloc_3d = mesh_3d.size() * sizeof(Vertex3D);
	size_t alloc_2d = mesh_2d.size() * sizeof(Vertex2D);

	// TODO FOR THE LOVE OF GOD ALMIGHTY GET RID OF THIS HACK! FOR NOW
	// TODO IT IS NEEDED TO STOP VALIDATION CRASHES WHEN BUFFER IS EMPTY.
	// TODO WELL, THIS WHOLE METHOD IS TRASH ACTUALLY
	if (alloc_3d == 0) alloc_3d ++;
	if (alloc_2d == 0) alloc_2d ++;

	if ((int) mesh_3d.size() > *len_3d) {
		BufferInfo buffer_builder {alloc_3d, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
		buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

		if (*len_3d != -1) {
			buf_3d->close();
		}

		*buf_3d = allocator.allocateBuffer(buffer_builder);
		logger::info("Reallocated 3D immediate buffer ", mesh_3d.size());
	}

	MemoryMap map_3d = buf_3d->access().map();
	map_3d.write(mesh_3d.data(), mesh_3d.size() * sizeof(Vertex3D));
	map_3d.flush();
	map_3d.unmap();
	*len_3d = mesh_3d.size();

	if ((int) mesh_2d.size() > *len_2d) {
		BufferInfo buffer_builder {alloc_2d, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
		buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

		if (*len_2d != -1) {
			buf_2d->close();
		}

		*buf_2d = allocator.allocateBuffer(buffer_builder);
		logger::info("Reallocated 2D immediate buffer ", mesh_2d.size());
	}

	MemoryMap map_2d = buf_2d->access().map();
	map_2d.write(mesh_2d.data(), mesh_2d.size() * sizeof(Vertex2D));
	map_2d.flush();
	map_2d.unmap();
	*len_2d = mesh_2d.size();

}