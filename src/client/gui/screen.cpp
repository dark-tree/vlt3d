
#include "screen.hpp"

Screen::Screen() {
	this->state = OPEN;
	this->replacement = nullptr;
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

void Screen::draw(RenderSystem& system, ImmediateRenderer& renderer, InputContext& input, Camera& camera, bool focused) {
	// override to implement custom behavior
}