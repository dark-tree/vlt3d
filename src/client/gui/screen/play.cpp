
#include "play.hpp"
#include "util/math/random.hpp"
#include "world/skybox.hpp"

PlayScreen::PlayScreen(World& world, Camera& camera)
: world(world), camera(camera) {}

InputResult PlayScreen::onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) {

	if (auto key = event.as<KeyboardEvent>()) {

		if (key->isKeyPressed(GLFW_KEY_R)) {
			if (Raycast raycast = world.raycast(camera.getPosition(), {0, -1, 0}, 1000)) {
				camera.move(raycast.getPos() + glm::ivec3 {0, 1, 0});
			}
		}

	}

	return InputResult::BLOCK;
}

void PlayScreen::draw(ImmediateRenderer& immediate, InputContext& input, Camera& camera, bool focused) {

	if (focused) {
		camera.update();

		Block air {0};
		Block idk {2};

		if (input.isRightPressed()) {
			if (Raycast raycast = world.raycast(camera.getPosition(), camera.getDirection(), 25.0f)) {
				glm::ivec3 pos = raycast.getPos();

				if (world.getBlock(pos.x, pos.y, pos.z) != air) {
					world.setBlock(pos.x, pos.y, pos.z, air);
				}
			}
		}

		if (input.isLeftPressed()) {
			if (Raycast raycast = world.raycast(camera.getPosition(), camera.getDirection(), 25.0f)) {
				glm::ivec3 pos = raycast.getTarget();

				if (world.getBlock(pos.x, pos.y, pos.z) != idk) {
					world.setBlock(pos.x, pos.y, pos.z, idk);
				}
			}
		}
	}

	immediate.setTint(255, 255, 255, 100);
	immediate.drawCircle(immediate.getWidth() / 2, immediate.getHeight() / 2, 2);

	Skybox skybox;
	skybox.draw(immediate, camera);

}