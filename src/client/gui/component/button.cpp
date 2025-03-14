
#include "button.hpp"
#include "client/immediate.hpp"
#include "client/gui/grid/context.hpp"

GuiButton::GuiButton(Box2D box, std::string text, std::string identifier, std::function<void(ScreenStack&)> callback)
: GuiComponent(box), text(std::move(text)), identifier(std::move(identifier)), callback(std::move(callback)) {}

void GuiButton::draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) {
	Box2D box = grid.getScreenBox(bounding);
	glm::vec2 extend = {box.width(), box.height()};

	int row = input.isMouseWithin(box) ? 1 + input.isLeftPressed() : 0;

	// handle navigator input
	if (grid.isFocused(this) && input.isKeyPressed(GLFW_KEY_ENTER)) {
		row = 2;
	}

	// draw button body
	renderer.setTint(255, 255, 255);
	renderer.drawBar(box.x1, box.y1, extend.x, extend.y, 1.0f, renderer.getSprite(identifier), 4, 4, row, extend.y);

	Color colors[3] = {{100, 100, 100}, {150, 150, 150}, {5, 5, 5}};

	// draw button label
	renderer.setTint(colors[row]);
	renderer.setFontSize(2);
	renderer.setFontTilt(0);
	renderer.setAlignment(VerticalAlignment::CENTER);
	renderer.setAlignment(HorizontalAlignment::CENTER);
	renderer.drawText(box.begin(), text, extend);

	// draw navigator outline
	if (grid.isFocused(this)) {
		renderer.setTint(255, 255, 255);
		renderer.drawBar(box.x1, box.y1, extend.x, extend.y, 1.0f, renderer.getSprite(identifier), 4, 4, 3, extend.y);
	}

	drawDebugOutline(grid, input, renderer, "GuiButton", 0, 255, 0);
}

bool GuiButton::onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) {
	Box2D box = grid.getScreenBox(bounding);

	if (!GridContext::shouldAccept(box, input, event)) {
		return false;
	}

	if (auto* mouse = event.as<ButtonEvent>()) {
		if (mouse->hasLeftClicked() ) {
			callback(stack);
			return true;
		}
	}

	if (auto* key = event.as<KeyboardEvent>()) {
		if (grid.isFocused(this) && key->isKeyReleased(GLFW_KEY_ENTER)) {
			callback(stack);
			return true;
		}
	}

	return false;
}

void GuiButton::navigatorUpdate(GridNavigator& grid) {
	grid.addNavigationNode(this);
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