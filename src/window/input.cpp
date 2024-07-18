
#include "input.hpp"
#include "window.hpp"

InputContext::InputContext(Window& window)
: window(window) {}

glm::vec2 InputContext::getMouse() {
	double x, y;
	window.getCursor(&x, &y);
	return {x, y};
}

bool InputContext::isMouseWithin(Box2D box) {
	return box.contains(getMouse());
}

bool InputContext::isKeyPressed(int key) {
	return window.isKeyPressed(key);
}

bool InputContext::isButtonPressed(int button) {
	return window.isButtonPressed(button);
}

bool InputContext::isLeftPressed() {
	return isButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
}

bool InputContext::isRightPressed() {
	return isButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
}

InputResult InputConsumer::onEvent(InputContext& context, const InputEvent& event) {
	return InputResult::PASS; // by default input consumer does nothing, override to implement custom behavior
}
