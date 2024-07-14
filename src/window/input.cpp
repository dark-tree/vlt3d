
#include "input.hpp"
#include "window.hpp"

InputContext::InputContext(Window& window)
: window(window) {}

glm::vec2 InputContext::getMouse() {
	double x, y;
	window.getCursor(&x, &y);
	return {x, y};
}

bool InputContext::isKeyPressed(int key) {
	return window.isKeyPressed(key);
}

bool InputContext::isButtonPressed(int button) {
	return window.isButtonPressed(button);
}

InputResult InputConsumer::onKey(InputContext& context, InputEvent key) {
	return InputResult::PASS; // by default input consumer does nothing, override to implement custom behavior
}

InputResult InputConsumer::onMouse(InputContext& context, InputEvent button) {
	return InputResult::PASS; // by default input consumer does nothing, override to implement custom behavior
}

InputResult InputConsumer::onScroll(InputContext& context, float scroll) {
	return InputResult::PASS; // by default input consumer does nothing, override to implement custom behavior
}