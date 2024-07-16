#pragma once

#include "external.hpp"
#include "client/gui/screen.hpp"
#include "client/gui/grid/context.hpp"

class GridScreen : public Screen {

	private:

		GridContext context {10, 10, 32};

	public:

		GridScreen();
		~GridScreen() = default;

		InputResult onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) override;
		void draw(ImmediateRenderer &renderer, InputContext& input, Camera& camera) override;

};
