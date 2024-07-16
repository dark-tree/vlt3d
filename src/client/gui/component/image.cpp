
#include "image.hpp"
#include "client/gui/grid/context.hpp"

GuiImage::GuiImage(Box2D box, const std::string& identifier)
: GuiComponent(box), identifier(identifier) {}

void GuiImage::draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) {
	Box2D box = grid.getScreenBox(bounding);

	if (input.isMouseWithin(box)) {
		renderer.setTint(50, 200, 50);
	} else {
		renderer.setTint(200, 50, 50);
	}

	renderer.drawSprite(box.begin(), box.width(), box.height(), renderer.getSprite(identifier));
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

ComponentProducer GuiImage::Builder::build() const {
	return [box = this->getBoundBox(), id = this->identifier] (int x, int y) {
		return new GuiImage {box.offset(x, y), id};
	};
}