
#include "play.hpp"

PlayScreen::PlayScreen(World& world, Camera& camera)
: world(world), camera(camera) {}

InputResult PlayScreen::onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) {
	return InputResult::BLOCK;
}

void PlayScreen::draw(ImmediateRenderer& immediate, InputContext& input, Camera& camera, bool focused) {
	glm::ivec3 pos = camera.getPosition() + camera.getDirection() * 10.0f;

	if (focused) {
		camera.update();

		if (input.isRightPressed()) {
			Block air {0};

			if (world.getBlock(pos.x, pos.y, pos.z) != air) {
				world.setBlock(pos.x, pos.y, pos.z, air);
			}
		}

		if (input.isLeftPressed()) {
			Block idk {0};

			if (world.getBlock(pos.x, pos.y, pos.z) != idk) {
				world.setBlock(pos.x, pos.y, pos.z, idk);
			}
		}
	}

	immediate.drawCircle(glm::vec3(pos), 0.1);
}