#pragma once

#include "external.hpp"
#include "window/input.hpp"
#include "screen.hpp"
#include "client/renderer.hpp"

class ScreenStack : public InputConsumer {

	private:

		std::mutex mutex;
		std::list<std::unique_ptr<Screen>> screens;

	public:

		/**
		 * Pass the events to all open screens, from top to bottom (FILO)
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

};
