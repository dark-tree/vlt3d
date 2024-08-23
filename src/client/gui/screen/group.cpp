
#include "group.hpp"

InputResult GroupScreen::onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) {
	bool pass = true;

	for (auto it = screens.begin(); it != screens.end();) {
		std::unique_ptr<Screen>& screen = *it;

		// move the iterator before calling the callback so that
		// calling replace() in it will not invalidate our iterator
		std::advance(it, 1);

		if (screen->state != Screen::OPEN) {
			continue;
		}

		InputResult result = screen->onEvent(stack, input, event);

		if (result != InputResult::PASS) {
			pass = false;
		}
	}

	return pass ? InputResult::PASS : InputResult::BLOCK;
}

void GroupScreen::draw(RenderSystem& system, ImmediateRenderer& renderer, InputContext& input, Camera& camera, bool focused) {
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

		screen->draw(system, renderer, input, camera, focused);
	}

	if (closed) {
		remove();
	}
}