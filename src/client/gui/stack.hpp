#pragma once

#include "external.hpp"
#include "window/input.hpp"
#include "screen.hpp"
#include "client/immediate.hpp"

class ScreenStack : public InputConsumer {

	private:

		std::recursive_mutex mutex;
		size_t opened = 0;
		std::list<std::unique_ptr<Screen>> screens;

	public:

		/**
		 * Pass the events to all open screens, from top to bottom (LIFO)
		 * if any screen along the ways consumes or blocks the event it is not
		 * send further down the chain
		 */
		InputResult onEvent(InputContext& context, const InputEvent& event) override;

		void draw(ImmediateRenderer& renderer, InputContext& input, Camera& camera);

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
