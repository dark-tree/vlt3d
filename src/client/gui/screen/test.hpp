#pragma once

#include "external.hpp"
#include "client/gui/screen.hpp"

class TestScreen : public Screen {

	private:

		bool test = true;

	public:

		~TestScreen() override = default;

		InputResult onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) override;
		void draw(ImmediateRenderer &renderer, InputContext& input, Camera& camera, bool focused) override;

};
