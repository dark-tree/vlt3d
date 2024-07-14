
#include "stack.hpp"

InputResult ScreenStack::onKey(InputContext& context, InputEvent key) {
	return forEach([&] (Screen* screen) { return screen->onKey(*this, context, key); });
}

InputResult ScreenStack::onMouse(InputContext& context, InputEvent button) {
	return forEach([&] (Screen* screen) { return screen->onMouse(*this, context, button); });
}

InputResult ScreenStack::onScroll(InputContext& context, float scroll) {
	return forEach([&] (Screen* screen) { return screen->onScroll(*this, context, scroll); });
}

void ScreenStack::draw(ImmediateRenderer& renderer, Camera& camera) {
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

		screen->draw(renderer, camera);
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
