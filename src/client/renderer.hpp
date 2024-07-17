#pragma once

#include "external.hpp"
#include "buffer/buffer.hpp"
#include "buffer/allocator.hpp"
#include "buffer/font.hpp"
#include "vertices.hpp"
#include "camera.hpp"
#include "util/color.hpp"

enum struct BillboardMode {
	ONE_AXIS,
	TWO_AXIS,
};

enum struct VerticalAlignment {
	TOP    = 0,
	CENTER = 1,
	BOTTOM = 2
};

enum struct HorizontalAlignment {
	LEFT   = 0,
	CENTER = 1,
	RIGHT  = 2
};

class ImmediateRenderer {

	private:

		std::vector<Vertex2D> mesh_2d;
		std::vector<Vertex3D> mesh_3d;
		BakedSprite blank;
		BillboardMode mode;
		Color color;
		glm::vec3 target;
		VerticalAlignment vertical;
		HorizontalAlignment horizontal;
		float font_size, line_size, font_tilt;
		uint32_t width, height;
		Atlas& atlas;
		Font& font;

		void drawBillboardVertex(glm::quat rotation, glm::vec3 offset, float x, float y, float u, float v);
		glm::quat getBillboardRotation(glm::vec3 center) const;
		glm::vec3 getPerpendicular(glm::vec3 normal, float angle) const;
		glm::vec2 getTextAlignment(const std::string& text, glm::vec2 extend) const;

	public:

		ImmediateRenderer(Atlas& atlas, Font& font);

		BakedSprite getSprite(const std::string& identifier);
		int getWidth() const;
		int getHeight() const;

		void setFontSize(float size);
		void setFontTilt(float tilt);
		void setLineSize(float size);
		void setTint(Color color);
		void setTint(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
		void setFacing(float x, float y, float z);
		void setFacing(glm::vec3 target);
		void setFacing(const WorldObject& object);
		void setBillboardMode(BillboardMode mode);
		void setAlignment(VerticalAlignment alignment);
		void setAlignment(HorizontalAlignment alignment);
		void setAlignment(VerticalAlignment vertical, HorizontalAlignment horizontal);

		// 2D
		void drawVertex(float x, float y, float u, float v);
		void drawVertex(glm::vec2 pos, float u, float v);
		void drawText(float x, float y, const std::string& text, glm::vec2 extend = {-1, -1});
		void drawText(glm::vec2 pos, const std::string& text, glm::vec2 extend = {-1, -1});
		void drawSprite(float x, float y, float w, float h, BakedSprite sprite);
		void drawSprite(glm::vec2 pos, float w, float h, BakedSprite sprite);
		void drawLine(float x1, float y1, float x2, float y2);
		void drawLine(glm::vec2 pa, glm::vec2 pb);

		// 3D
		void drawVertex(float x, float y, float z, float u, float v);
		void drawVertex(glm::vec3 pos, float u, float v);
		void drawText(float x, float y, float z, const std::string& text, glm::vec2 extend = {-1, -1});
		void drawText(glm::vec3 pos, const std::string& text, glm::vec2 extend = {-1, -1});
		void drawSprite(float x, float y, float z, float w, float h, BakedSprite sprite);
		void drawSprite(glm::vec3 pos, float w, float h, BakedSprite sprite);
		void drawLine(float x1, float y1, float z1, float x2, float y2, float z2);
		void drawLine(glm::vec3 pa, glm::vec3 pb);

		void prepare(VkExtent2D extend);
		void getBuffers(Allocator& allocator, Buffer* buf1, int* len1, Buffer* buf2, int* len2);

};