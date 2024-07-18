
#include "button.hpp"

#include <utility>
#include "client/gui/grid/context.hpp"

GuiButton::GuiButton(Box2D box, std::string text, std::string identifier, std::function<void(ScreenStack&)> callback)
: GuiComponent(box), text(std::move(text)), identifier(std::move(identifier)), callback(std::move(callback)) {}

void GuiButton::draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) {
	Box2D box = grid.getScreenBox(bounding);
	glm::vec2 extend = {box.width(), box.height()};

	int po = input.isButtonPressed(GLFW_MOUSE_BUTTON_LEFT) ? 1 : 0;
	int br = input.isMouseWithin(box) ? 1 + po : 0;
	int tc = input.isMouseWithin(box) ? 1 + po : 0;

	renderer.setTint(255, 255, 255);
	renderer.setAlignment(VerticalAlignment::CENTER);
	renderer.setAlignment(HorizontalAlignment::CENTER);
	renderer.drawBar(box.x1, box.y1, extend.x, extend.y, 1.0f, renderer.getSprite(identifier), 4, 4, br, extend.y);

	Color colors[3] = {{100, 100, 100}, {150, 150, 150}, {5, 5, 5}};

	renderer.setTint(colors[tc]);
	renderer.setFontSize(2);
	renderer.setFontTilt(0);
	renderer.drawText(box.begin(), text, extend);

	drawDebugOutline(grid, input, renderer, "GuiButton", 0, 255, 0);
}

bool GuiButton::onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) {
	Box2D box = grid.getScreenBox(bounding);

	if (!grid.shouldAccept(box, input, event)) {
		return false;
	}

	if (auto* mouse = event.as<MouseEvent>()) {
		if (mouse->hasLeftClicked() ) {
			callback(stack);
			return true;
		}
	}

	return false;
}

/*
 * GuiButton Builder
 */

GuiButton::Builder& GuiButton::Builder::text(const std::string& text) {
	this->string = text;
	return *this;
}

GuiButton::Builder& GuiButton::Builder::sprite(const std::string& identifier) {
	this->identifier = identifier;
	return *this;
}

GuiButton::Builder& GuiButton::Builder::then(std::function<void(ScreenStack&)> callback) {
	this->callback = callback;
	return *this;
}

ComponentProducer GuiButton::Builder::build() const {
	return [box = this->getBoundBox(), text = this->string, id = this->identifier, callback = this->callback] (int x, int y) {
		return new GuiButton {box.offset(x, y), text, id, callback};
	};
}