#pragma once

#include "external.hpp"
#include "client/gui/screen.hpp"
#include "world/world.hpp"

class PlayScreen : public Screen {

	private:

		World& world;
		Camera& camera;

	public:

		PlayScreen(World& world, Camera& camera);
		~PlayScreen() override = default;

		InputResult onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) override;
		void draw(RenderSystem& system, ImmediateRenderer &renderer, InputContext& input, Camera& camera, bool focused) override;

};
