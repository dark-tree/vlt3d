#pragma once

#include "external.hpp"
#include "client/gui/screen.hpp"
#include "client/gui/stack.hpp"

class GroupScreen : public Screen {

	private:

		std::list<std::unique_ptr<Screen>> screens;

	public:

		template <typename... Screens> requires trait::All<Screen*, Screens...>
		GroupScreen(Screens... screens) {
			( this->screens.emplace_back(screens), ... );
		}

		InputResult onEvent(ScreenStack& stack, InputContext& input, const InputEvent& key) override;
		void draw(ImmediateRenderer& renderer, InputContext& input, Camera& camera, bool focused) override;

};
