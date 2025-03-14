#pragma once

#include "external.hpp"
#include "client/gui/screen.hpp"
#include "window/profiler.hpp"

class TestScreen : public Screen {

	private:

		Profiler& profiler;
		bool test = true;

	public:

		TestScreen(Profiler& profiler);
		~TestScreen() override = default;

		InputResult onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) override;
		void draw(ImmediateRenderer &renderer, InputContext& input, Camera& camera, bool focused) override;

};
