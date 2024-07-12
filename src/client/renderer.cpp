
#include "renderer.hpp"
#include "client/vertices.hpp"
#include "buffer/allocator.hpp"
#include "buffer/atlas.hpp"

ImmediateRenderer::ImmediateRenderer(Atlas& atlas, Font& font)
: atlas(atlas), font(font) {
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
	setFacing({x, y, z});
}

void ImmediateRenderer::setFacing(glm::vec3 facing) {
	this->facing = facing;
}

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

	float ex = sx + w;
	float ey = sy + h;

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

void ImmediateRenderer::getBuffers(Allocator& allocator, Buffer* buf_3d, int* len_3d, Buffer* buf_2d, int* len_2d, VkExtent2D extent) {

	float t = glfwGetTime() * 1.33;
	float ox = sin(t);
	float oy = cos(t);

	this->width = extent.width / 2;
	this->height = extent.height / 2;
	mesh_2d.clear();
	mesh_3d.clear();

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

	mesh_3d.emplace_back( ox + 0, oy + 0,  -1,   0,   0, 255,   0,   0, 255);
	mesh_3d.emplace_back( ox + 1, oy + 0,  -1,   1,   0,   0, 255,   0, 255);
	mesh_3d.emplace_back( ox + 1, oy + 1,  -1,   1,   1,   0,   0, 255, 255);

	mesh_3d.emplace_back( ox + 0, oy + 0,  -1,   0,   0, 255,   0,   0, 255);
	mesh_3d.emplace_back( ox + 1, oy + 1,  -1,   1,   1,   0,   0, 255, 255);
	mesh_3d.emplace_back( ox + 0, oy + 1,  -1,   0,   1,   0, 255,   0, 255);

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