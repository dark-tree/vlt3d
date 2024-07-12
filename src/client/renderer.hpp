#pragma once

#include "external.hpp"
#include "buffer/buffer.hpp"
#include "buffer/allocator.hpp"
#include "buffer/font.hpp"
#include "vertices.hpp"

class ImmediateRenderer {

	private:

		std::vector<Vertex2D> mesh_2d;
		std::vector<Vertex3D> mesh_3d;
		BakedSprite blank;
		uint8_t r, g, b, a;
		glm::vec3 facing;
		float font_size, line_size;
		uint32_t width, height;
		Atlas& atlas;
		Font& font;

	public:

		ImmediateRenderer(Atlas& atlas, Font& font);

		BakedSprite getSprite(const std::string& identifier);
		void setFontSize(float size);
		void setLineSize(float size);
		void setTint(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
		void setFacing(float x, float y, float z);
		void setFacing(glm::vec3 facing);
		void setFacingCamera();

		// 2D
		void drawVertex(float x, float y, float u, float v);
		void drawVertex(glm::vec2 pos, float u, float v);
		void drawText(float x, float y, const std::string& text);
		void drawText(glm::vec2 pos, const std::string& text);
		void drawSprite(float x, float y, float w, float h, BakedSprite sprite);
		void drawSprite(glm::vec2 pos, float w, float h, BakedSprite sprite);
		void drawLine(float x1, float y1, float x2, float y2);
		void drawLine(glm::vec2 pa, glm::vec2 pb);

		// 3D
		void drawVertex(float x, float y, float z, float u, float v);
		void drawVertex(glm::vec3 pos, float u, float v);
		void drawText(float x, float y, float z, const std::string& text);
		void drawText(glm::vec3 pos, const std::string& text);
		void drawSprite(float x, float y, float z, float w, float h, BakedSprite sprite);
		void drawSprite(glm::vec3 pos, float w, float h, BakedSprite sprite);
		void drawLine(float x1, float y1, float z1, float x2, float y2, float z2);
		void drawLine(glm::vec3 pa, glm::vec3 pb);

		void getBuffers(Allocator& allocator, Buffer* buf1, int* len1, Buffer* buf2, int* len2, VkExtent2D extend);

};