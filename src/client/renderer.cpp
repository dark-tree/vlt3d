
#include "renderer.hpp"
#include "client/vertices.hpp"
#include "buffer/allocator.hpp"
#include "buffer/atlas.hpp"

ImmediateRenderer::ImmediateRenderer(Atlas& atlas, Font& font)
: mode(BillboardMode::TWO_AXIS), atlas(atlas), font(font) {
	this->blank = getSprite("assets/sprites/blank.png");
}

BakedSprite ImmediateRenderer::getSprite(const std::string& identifier) {
	return atlas.getBakedSprite(identifier);
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
	float xs = font_size;
	float ys = font_size;

	for (char chr : text) {

		Glyph glyph = font.getGlyph(chr);
		BakedSprite sprite = glyph.getSprite();

		float w = glyph.getWidth() * xs;

		float sx = x + offset;
		float ex = sx + w;
		float sy = y;
		float ey = sy + glyph.getHeight() * ys;

		drawVertex(sx, sy, sprite.u1, sprite.v1);
		drawVertex(ex, ey, sprite.u2, sprite.v2);
		drawVertex(sx, ey, sprite.u1, sprite.v2);

		drawVertex(sx, sy, sprite.u1, sprite.v1);
		drawVertex(ex, sy, sprite.u2, sprite.v1);
		drawVertex(ex, ey, sprite.u2, sprite.v2);

		offset += w + xs;

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
	float xs = font_size;
	float ys = font_size;
	glm::quat rot = getBillboardRotation(pos);

	float total = 0;

	for (char chr : text) {
		total += (font.getGlyph(chr).getWidth() + 1) * xs;
	}

	for (char chr : text) {

		Glyph glyph = font.getGlyph(chr);
		BakedSprite sprite = glyph.getSprite();

		float w = glyph.getWidth() * xs;
		float h = glyph.getHeight() * ys;

		float sx = offset - total / 2;
		float ex = sx + w;
		float sy = -h / 2;
		float ey = sy + h;

		drawBillboardVertex(rot, pos, sx, sy, sprite.u1, sprite.v1);
		drawBillboardVertex(rot, pos, ex, ey, sprite.u2, sprite.v2);
		drawBillboardVertex(rot, pos, sx, ey, sprite.u1, sprite.v2);

		drawBillboardVertex(rot, pos, sx, sy, sprite.u1, sprite.v1);
		drawBillboardVertex(rot, pos, ex, sy, sprite.u2, sprite.v1);
		drawBillboardVertex(rot, pos, ex, ey, sprite.u2, sprite.v2);

		offset += w + xs;

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

void ImmediateRenderer::getBuffers(Allocator& allocator, Buffer* buf_3d, int* len_3d, Buffer* buf_2d, int* len_2d, VkExtent2D extent, Camera& camera) {

	float t = glfwGetTime() * 1.33;
	float ox = sin(t);
	float oy = cos(t);

	this->width = extent.width / 2;
	this->height = extent.height / 2;
	mesh_2d.clear();
	mesh_3d.clear();

	setFacing(camera);

	setTint(255, 255, 255);
	setFontSize(2);
	setLineSize(4);
	drawSprite(10, 10, 100, 100, getSprite("assets/sprites/vkblob.png"));
	drawText(10, 10, "Hello Bitmap Font!");

	setTint(10, 100, 220);
	drawLine(50, 200, 50, 550);
	drawLine(50, 550, 150, 650);
	drawLine(150, 650, 900, 650);
	drawLine(300, 300, 300 + ox * 150, 300 + oy * 150);

	setTint(0, 0, 255);
	setFontSize(0.05);
	setLineSize(0.05);
	drawSprite(10 * ox + 10, -3, 10 * oy + 10, 1, 1, getSprite("assets/sprites/vkblob.png"));
	drawText(0, 0, 0, "Hello!");
	drawLine(0, -3, 0, 10 * ox + 10, -3, 10 * oy + 10);

	setTint(255, 255, 0);
	setLineSize(0.08);
	drawLine(0 - 0.5, 0 - 0.5, 0 - 0.5, 0 - 0.5, -32 - 0.5, 0 - 0.5);
	drawLine(32 - 0.5, 0 - 0.5, 0 - 0.5, 32 - 0.5, -32 - 0.5, 0 - 0.5);
	drawLine(0 - 0.5, 0 - 0.5, 32 - 0.5, 0 - 0.5, -32 - 0.5, 32 - 0.5);
	drawLine(32 - 0.5, 0 - 0.5, 32 - 0.5, 32 - 0.5, -32 - 0.5, 32 - 0.5);
	drawLine(0 - 0.5, 0 - 0.5, 0 - 0.5, 32 - 0.5, 0 - 0.5, 0 - 0.5);
	drawLine(0 - 0.5, 0 - 0.5, 0 - 0.5, 0 - 0.5, 0 - 0.5, 32 - 0.5);
	drawLine(0 - 0.5, -32 - 0.5, 0 - 0.5, 32 - 0.5, -32 - 0.5, 0 - 0.5);
	drawLine(0 - 0.5, -32 - 0.5, 0 - 0.5, 0 - 0.5, -32 - 0.5, 32 - 0.5);
	drawLine(32 - 0.5, 0 - 0.5, 0 - 0.5, 32 - 0.5, 0 - 0.5, 32 - 0.5);
	drawLine(0 - 0.5, 0 - 0.5, 32 - 0.5, 32 - 0.5, 0 - 0.5, 32 - 0.5);
	drawLine(32 - 0.5, -32 - 0.5, 0 - 0.5, 32 - 0.5, -32 - 0.5, 32 - 0.5);
	drawLine(0 - 0.5, -32 - 0.5, 32 - 0.5, 32 - 0.5, -32 - 0.5, 32 - 0.5);

	if ((int) mesh_3d.size() > *len_3d) {
		BufferInfo buffer_builder {mesh_3d.size() * sizeof(Vertex3D), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
		buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

		if (*len_3d != -1) {
			buf_3d->close();
		}

		*buf_3d = allocator.allocateBuffer(buffer_builder);
		*len_3d = mesh_3d.size();
		logger::info("Reallocated 3D immediate buffer");
	}

	MemoryMap map_3d = buf_3d->access().map();
	map_3d.write(mesh_3d.data(), mesh_3d.size() * sizeof(Vertex3D));
	map_3d.flush();
	map_3d.unmap();

	if ((int) mesh_2d.size() > *len_2d) {
		BufferInfo buffer_builder {mesh_2d.size() * sizeof(Vertex2D), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
		buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

		if (*len_2d != -1) {
			buf_2d->close();
		}

		*buf_2d = allocator.allocateBuffer(buffer_builder);
		*len_2d = mesh_2d.size();
		logger::info("Reallocated 2D immediate buffer");
	}

	MemoryMap map_2d = buf_2d->access().map();
	map_2d.write(mesh_2d.data(), mesh_2d.size() * sizeof(Vertex2D));
	map_2d.flush();
	map_2d.unmap();
}