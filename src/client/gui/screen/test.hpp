#pragma once

#include "external.hpp"
#include "client/gui/screen.hpp"

class TestScreen : public Screen {

	private:

		bool test = true;

	public:

		~TestScreen() override;

		InputResult onKey(ScreenStack& stack, InputContext& input, InputEvent key) override;
		void draw(ImmediateRenderer &renderer, Camera& camera) override;

};
