#pragma once

#include "external.hpp"
#include "window/input.hpp"
#include "client/immediate.hpp"

class ScreenStack;

class Screen {

	public:

		enum State {
			OPEN,
			REMOVED,
			REPLACED
		};

	private:

		State state;
		Screen* replacement;

		friend class ScreenStack;
		friend class GroupScreen;

	public:

		Screen();
		virtual ~Screen() = default;

		virtual InputResult onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event);
		virtual void draw(ImmediateRenderer& renderer, InputContext& input, Camera& camera, bool focused);

		/**
		 * Close this screen, no more events and draw calls
		 * will be issued to this screen after this method is called
		 */
		void remove();

		/**
		 * Removes this screen and replaces it with a different one, the
		 * effect of calling this method for this screen is the same as remove()
		 */
		void replace(Screen* screen);

};
