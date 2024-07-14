
#include "stack.hpp"

InputResult ScreenStack::onKey(InputContext& context, InputEvent key) {
	std::unique_lock lock {mutex};

	for (auto& screen : screens) {
		if (screen->removed) {
			continue;
		}

		InputResult result = screen->onKey(context, key);

		if (result != InputResult::PASS) {
			return result;
		}
	}

	return InputResult::PASS;
}

InputResult ScreenStack::onMouse(InputContext& context, InputEvent button) {
	std::unique_lock lock {mutex};

	for (auto& screen : screens) {
		if (screen->removed) {
			continue;
		}

		InputResult result = screen->onMouse(context, button);

		if (result != InputResult::PASS) {
			return result;
		}
	}

	return InputResult::PASS;
}

InputResult ScreenStack::onScroll(InputContext& context, float scroll) {
	std::unique_lock lock {mutex};

	for (auto& screen : screens) {
		if (screen->removed) {
			continue;
		}

		InputResult result = screen->onScroll(context, scroll);

		if (result != InputResult::PASS) {
			return result;
		}
	}

	return InputResult::PASS;
}

void ScreenStack::draw(ImmediateRenderer& renderer, Camera& camera) {
	std::unique_lock lock {mutex};

	for (auto it = screens.rbegin(); it != screens.rend();) {
		auto& screen = *it;

		if (screen->removed) {
			logger::info("Removed screen ", screen.get());
			it = decltype(it) {screens.erase(std::next(it).base())};
			continue;
		}

		screen->draw(renderer, camera);
		std::advance(it, 1);
	}
}

void ScreenStack::open(Screen* screen) {
	std::unique_lock lock {mutex};
	screens.emplace_front(screen);
}

void ScreenStack::close() {
	std::unique_lock lock {mutex};
	screens.clear();
}


