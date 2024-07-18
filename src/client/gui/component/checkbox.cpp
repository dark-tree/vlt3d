
#include "button.hpp"

#include "checkbox.hpp"
#include "client/renderer.hpp"
#include "client/gui/grid/context.hpp"

GuiCheck::GuiCheck(Box2D box, std::string label, std::function<void(bool)> callback, bool initial)
: GuiComponent(box), label(std::move(label)), callback(std::move(callback)), state(initial) {}

void GuiCheck::draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) {
	Box2D box = grid.getScreenBox(bounding);
	glm::vec2 extend = {box.width(), box.height()};

	int column = state;
	int row = input.isMouseWithin(box) ? 1 + input.isLeftPressed() : 0;

	// handle navigator input
	if (grid.isFocused(this) && input.isKeyPressed(GLFW_KEY_ENTER)) {
		row = 2;
	}

	// draw label
	renderer.setFontSize(2);
	renderer.setFontTilt(0);
	renderer.setTint(255, 255, 255);
	renderer.setAlignment(VerticalAlignment::CENTER);
	renderer.setAlignment(HorizontalAlignment::LEFT);
	renderer.drawText(box.x2 + 4, box.y1, label, {-1, box.height()});

	// draw main checkbox
	BakedSprite checkbox = renderer.getSprite("assets/sprites/checkbox.png");
	renderer.drawSprite(box.begin(), extend.x, extend.y, checkbox.grid(4, 4, row, column));

	// draw navigator outline
	if (grid.isFocused(this)) {
		renderer.drawSprite(box.begin(), extend.x, extend.y, checkbox.grid(4, 4, 3, column));
	}

	drawDebugOutline(grid, input, renderer, "GuiCheck", 4, 200, 100);
}

bool GuiCheck::onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) {
	Box2D box = grid.getScreenBox(bounding);

	if (!GridContext::shouldAccept(box, input, event)) {
		return false;
	}

	if (auto* mouse = event.as<MouseEvent>()) {
		if (mouse->hasLeftClicked()) {
			state = !state;
			callback(state);
			return true;
		}
	}

	if (auto* key = event.as<KeyboardEvent>()) {
		if (grid.isFocused(this) && key->isKeyReleased(GLFW_KEY_ENTER)) {
			state = !state;
			callback(state);
			return true;
		}
	}

	return false;
}

void GuiCheck::navigatorUpdate(GridNavigator& grid) {
	grid.addNavigationNode(this);
}

/*
 * GuiButton Builder
 */

GuiCheck::Builder& GuiCheck::Builder::label(const std::string& text) {
	this->string = text;
	return *this;
}

GuiCheck::Builder& GuiCheck::Builder::then(std::function<void(bool)> callback) {
	this->callback = callback;
	return *this;
}

GuiCheck::Builder& GuiCheck::Builder::initial(bool value) {
	this->value = value;
	return *this;
}

ComponentProducer GuiCheck::Builder::build() const {
	return [box = this->getBoundBox(), label = this->string, callback = this->callback, value = this->value] (int x, int y) {
		return new GuiCheck {box.offset(x, y), label, callback, value};
	};
}