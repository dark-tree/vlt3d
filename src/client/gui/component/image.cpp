
#include "image.hpp"
#include "client/gui/grid/context.hpp"

GuiImage::GuiImage(Box2D box, const std::string& identifier, Color color)
: GuiComponent(box), identifier(identifier), color(color) {}

void GuiImage::draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) {
	Box2D box = grid.getScreenBox(bounding);

	renderer.setTint(color);
	renderer.drawSprite(box.begin(), box.width(), box.height(), renderer.getSprite(identifier));
	drawDebugOutline(grid, input, renderer, "GuiImage", 255, 0, 0);
}

bool GuiImage::onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) {
	return false;
}

/*
 * GuiImage Builder
 */

GuiImage::Builder& GuiImage::Builder::sprite(const std::string& identifier) {
	this->identifier = identifier;
	return *this;
}

GuiImage::Builder& GuiImage::Builder::tint(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	this->color = Color {r, g, b, a};
	return *this;
}

ComponentProducer GuiImage::Builder::build() const {
	return [box = this->getBoundBox(), id = this->identifier, color = this->color] (int x, int y) {
		return new GuiImage {box.offset(x, y), id, color};
	};
}