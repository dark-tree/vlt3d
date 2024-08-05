
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
			if (world.getBlock(pos.x, pos.y, pos.z) != 0) {
				world.setBlock(pos.x, pos.y, pos.z, 0);
			}
		}

		if (input.isLeftPressed()) {
			if (world.getBlock(pos.x, pos.y, pos.z) != 1) {
				world.setBlock(pos.x, pos.y, pos.z, 1);
			}
		}
	}

	immediate.drawCircle(glm::vec3(pos), 0.1);
}