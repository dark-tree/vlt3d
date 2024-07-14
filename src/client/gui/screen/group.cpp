
#include "group.hpp"

InputResult GroupScreen::onKey(ScreenStack& stack, InputContext& input, InputEvent key) {
	return forEach([&] (Screen* screen) { return screen->onKey(stack, input, key); });
}

InputResult GroupScreen::onMouse(ScreenStack& stack, InputContext& input, InputEvent button) {
	return forEach([&] (Screen* screen) { return screen->onMouse(stack, input, button); });
}

InputResult GroupScreen::onScroll(ScreenStack& stack, InputContext& input, float scroll) {
	return forEach([&] (Screen* screen) { return screen->onScroll(stack, input, scroll); });
}

void GroupScreen::draw(ImmediateRenderer& renderer, Camera& camera) {
	bool closed = false;

	for (auto& screen : screens) {
		if (screen->state == Screen::REPLACED) {
			screen.reset(screen->replacement);
			continue;
		}

		if (screen->state == Screen::REMOVED) {
			closed = true;
			break;
		}

		screen->draw(renderer, camera);
	}

	if (closed) {
		remove();
	}
}