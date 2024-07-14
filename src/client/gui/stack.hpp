#pragma once

#include "external.hpp"
#include "window/input.hpp"
#include "screen.hpp"
#include "client/renderer.hpp"

class ScreenStack : public InputConsumer {

	private:

		std::recursive_mutex mutex;
		size_t opened = 0;
		std::list<std::unique_ptr<Screen>> screens;

		template <typename F>
		InputResult forEach(F function) {
			std::lock_guard lock {mutex};

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
					return result;
				}
			}

			return InputResult::PASS;
		}

	public:

		/**
		 * Pass the events to all open screens, from top to bottom (LIFO)
		 * if any screen along the ways consumes or blocks the event it is not
		 * send further down the chain
		 */
		InputResult onKey(InputContext& context, InputEvent key) override;
		InputResult onMouse(InputContext& context, InputEvent button) override;
		InputResult onScroll(InputContext& context, float scroll) override;

		void draw(ImmediateRenderer& renderer, Camera& camera);

	public:

		/**
		 * Takes a raw pointer to a newly allocated Screen instance,
		 * the screen stack will then take ownership of the pointer
		 */
		void open(Screen* screen);

		/**
		 * Close all screens in this stack,
		 * and deallocates their memory
		 */
		void close();

		/**
		 * Get the number of open screens in this
		 * stack
		 */
		 int count() const;

};
