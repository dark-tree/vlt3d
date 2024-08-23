
#include "stack.hpp"

InputResult ScreenStack::onEvent(InputContext& input, const InputEvent& event) {
	std::lock_guard lock {mutex};

	for (auto it = screens.begin(); it != screens.end();) {
		std::unique_ptr<Screen>& screen = *it;

		// move the iterator before calling the callback so that
		// calling replace() in it will not invalidate our iterator
		std::advance(it, 1);

		if (screen->state != Screen::OPEN) {
			continue;
		}

		InputResult result = screen->onEvent(*this, input, event);

		if (result != InputResult::PASS) {
			return result;
		}
	}

	return InputResult::PASS;
}

void ScreenStack::draw(RenderSystem& system, ImmediateRenderer& renderer, InputContext& input, Camera& camera) {
	std::lock_guard lock {mutex};

	for (auto it = screens.rbegin(); it != screens.rend();) {
		auto& screen = *it;

		if (screen->state == Screen::REMOVED) {
			logger::info("Removed screen ", screen.get());
			opened --;
			it = decltype(it) {screens.erase(std::next(it).base())};
			continue;
		}

		if (screen->state == Screen::REPLACED) {
			logger::info("Replaced screen ", screen.get(), " with ", screen->replacement);
			screen.reset(screen->replacement);
			std::advance(it, 1);
			continue;
		}

		screen->draw(system, renderer, input, camera, next(it) == screens.rend());
		std::advance(it, 1);
	}
}

void ScreenStack::open(Screen* screen) {
	std::lock_guard lock {mutex};
	screens.emplace_front(screen);
	opened ++;
}

void ScreenStack::close() {
	std::lock_guard lock {mutex};

	for (auto const& screen : screens) {
		screen->remove();
	}
}

int ScreenStack::count() const {
	return opened;
}
