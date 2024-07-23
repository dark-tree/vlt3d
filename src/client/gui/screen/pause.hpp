#pragma once

#include "grid.hpp"

class PauseScreen : public GridScreen {

	public:

		PauseScreen();

	private:

		virtual void buildModel(GuiComposed::Builder& builder) override;

		InputResult onEvent(ScreenStack &stack, InputContext &input, const InputEvent &event) override;
		void draw(ImmediateRenderer &renderer, InputContext &input, Camera &camera, bool focused) override;
};
