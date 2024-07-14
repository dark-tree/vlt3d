#pragma once

#include "external.hpp"
#include "client/gui/screen.hpp"
#include "client/gui/stack.hpp"

class GroupScreen : public Screen {

	private:

		std::list<std::unique_ptr<Screen>> screens;

		template <typename F>
		InputResult forEach(F function) {
			bool pass = true;

			for (auto it = screens.begin(); it != screens.end();) {
				std::unique_ptr<Screen>& screen = *it;

				// move the iterator before calling the callback so that
				// calling replace() in it will not invalidate our iterator
				std::advance(it, 1);

				if (screen->state != Screen::OPEN) {
					continue;
				}

				InputResult result = function(screen.get());

				if (result != InputResult::PASS) {
					pass = false;
				}
			}

			return pass ? InputResult::PASS : InputResult::BLOCK;
		}

	public:

		template <typename... Screens> requires trait::All<Screen*, Screens...>
		GroupScreen(Screens... screens) {
			( this->screens.emplace_back(screens), ... );
		}

		InputResult onKey(ScreenStack& stack, InputContext& input, InputEvent key);
		InputResult onMouse(ScreenStack& stack, InputContext& input, InputEvent button);
		InputResult onScroll(ScreenStack& stack, InputContext& input, float scroll);
		void draw(ImmediateRenderer& renderer, Camera& camera);

};
