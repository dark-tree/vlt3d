
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

InputResult Screen::onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) {
	return InputResult::PASS; // override to implement custom behavior
}

void Screen::draw(ImmediateRenderer& renderer, InputContext& input, Camera& camera) {
	// override to implement custom behavior
}