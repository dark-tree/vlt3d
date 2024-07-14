
#include "screen.hpp"

Screen::Screen() {
	this->state = OPEN;
}

void Screen::remove() {
	if (state == OPEN) {
		this->state = REMOVED;
		this->replacement = nullptr;
	}
}

void Screen::replace(Screen* screen) {
	if (state == OPEN) {
		this->state = REPLACED;
		this->replacement = screen;
	}
}

InputResult Screen::onKey(ScreenStack& stack, InputContext& input, InputEvent key) {
	return InputResult::PASS; // override to implement custom behavior
}

InputResult Screen::onMouse(ScreenStack& stack, InputContext& input, InputEvent button) {
	return InputResult::PASS; // override to implement custom behavior
}

InputResult Screen::onScroll(ScreenStack& stack, InputContext& input, float scroll) {
	return InputResult::PASS; // override to implement custom behavior
}

void Screen::draw(ImmediateRenderer& renderer, Camera& camera) {
	// override to implement custom behavior
}