
#include "test.hpp"
#include "pause.hpp"
#include "client/renderer.hpp"

TestScreen::TestScreen(Profiler& profiler)
: profiler(profiler) {}

InputResult TestScreen::onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) {
	if (auto* key = event.as<KeyboardEvent>()) {

		if (key->isKeyReleased(GLFW_KEY_SPACE)) {
			test = !test;
			return InputResult::CONSUME;
		}

		if (key->isKeyReleased(GLFW_KEY_ESCAPE)) {
			stack.open(new PauseScreen {});
			return InputResult::CONSUME;
		}

	}

	return InputResult::PASS;
}

void TestScreen::draw(RenderSystem& system, ImmediateRenderer& renderer, InputContext& input, Camera& camera, bool focused) {

	AllocationArena::Stats stats;

	{
		std::lock_guard lock{unified_mutex};
		stats = system.unified_buffer.arena.getStats();
	}

	if (focused) {
		input.setMouseCapture(true);
	}

	if (!test) {
		renderer.setAlignment(HorizontalAlignment::LEFT);
		renderer.setAlignment(VerticalAlignment::TOP);
		renderer.setTint(255, 255, 0);
		renderer.setFontSize(2);
		renderer.drawText(10, 10 + 9 * 0, "Press [SPACE] to show");
		renderer.drawText(10, 10 + 9 * 2, "Press [ESCAPE] to close");
		return;
	}

	float t = glfwGetTime() * 1.33;
	float ox = sin(t);
	float oy = cos(t);

	renderer.setTint(255, 255, 255);
	renderer.drawPatch(renderer.getWidth() - 160 - 32, 32, 10, 10, 16, renderer.getNinePatch("gui", 8));

	renderer.setFacing(camera);
	renderer.setFontSize(2);
	renderer.setLineSize(4);
	renderer.setAlignment(HorizontalAlignment::LEFT);
	renderer.setAlignment(VerticalAlignment::TOP);
	renderer.setTint(255, 255, 0);

	glm::vec3 pos = camera.getPosition();
	renderer.drawText(10, 10, "FPS: " + std::to_string(profiler.getCountPerSecond()));
	renderer.drawText(10, 10 + 9 * 2, "X: " + std::to_string(pos.x) + ", Y: " + std::to_string(pos.y) + ", Z: " + std::to_string(pos.z));
	renderer.drawText(10, 10 + 9 * 4, "U: " + std::to_string(stats.used / 1024) + " KiB, R: " + std::to_string(stats.reclaimed / 1024) + " KiB, F: " + std::to_string(stats.free / 1024) + " KiB, in " + std::to_string(stats.blocks) + " blocks");

	renderer.drawText(10, 10 + 9 * 6, "Press [SPACE] to hide");
	renderer.drawText(10, 10 + 9 * 8, "Press [ESCAPE] to close");

	renderer.setTint(200, 50, 50, 80);
	renderer.drawSprite(10, renderer.getHeight() - 30, renderer.getWidth() - 20, 20, renderer.getSprite("blank"));

	for (AllocationArena::Range range : stats.ranges) {
		double start = range.start / (double) stats.total;
		double end = range.end / (double) stats.total;

		start *= (renderer.getWidth() - 20);
		end *= (renderer.getWidth() - 20);

		renderer.setTint(50, 200, 50, 80);
		renderer.drawSprite(10 + start, renderer.getHeight() - 30, end - start, 20, renderer.getSprite("blank"));

		renderer.setTint(50, 200, 50, 80);
		renderer.drawSprite(10 + start, renderer.getHeight() - 50, 1, 20, renderer.getSprite("blank"));
	}

	renderer.setTint(255, 255, 255);
	renderer.setFontSize(0.05);
	renderer.setLineSize(0.05);
	renderer.drawTiled(10 * ox + 10, -3, 10 * oy + 10, 2.6, 2.6, renderer.getSprite("vkblob"), 1, 1);

	renderer.setAlignment(VerticalAlignment::CENTER);
	renderer.setAlignment(HorizontalAlignment::CENTER);
	renderer.drawText(-0.5, -0.5, -0.5, "Hello !");
	renderer.drawLine(0, -3, 0, 10 * ox + 10, -3, 10 * oy + 10);

	renderer.setTint(255, 255, 0);
	renderer.setLineSize(0.08);
	renderer.drawLine(0 - 0.5, 0 - 0.5, 0 - 0.5, 0 - 0.5, -32 - 0.5, 0 - 0.5);
	renderer.drawLine(32 - 0.5, 0 - 0.5, 0 - 0.5, 32 - 0.5, -32 - 0.5, 0 - 0.5);
	renderer.drawLine(0 - 0.5, 0 - 0.5, 32 - 0.5, 0 - 0.5, -32 - 0.5, 32 - 0.5);
	renderer.drawLine(32 - 0.5, 0 - 0.5, 32 - 0.5, 32 - 0.5, -32 - 0.5, 32 - 0.5);
	renderer.drawLine(0 - 0.5, 0 - 0.5, 0 - 0.5, 32 - 0.5, 0 - 0.5, 0 - 0.5);
	renderer.drawLine(0 - 0.5, 0 - 0.5, 0 - 0.5, 0 - 0.5, 0 - 0.5, 32 - 0.5);
	renderer.drawLine(0 - 0.5, -32 - 0.5, 0 - 0.5, 32 - 0.5, -32 - 0.5, 0 - 0.5);
	renderer.drawLine(0 - 0.5, -32 - 0.5, 0 - 0.5, 0 - 0.5, -32 - 0.5, 32 - 0.5);
	renderer.drawLine(32 - 0.5, 0 - 0.5, 0 - 0.5, 32 - 0.5, 0 - 0.5, 32 - 0.5);
	renderer.drawLine(0 - 0.5, 0 - 0.5, 32 - 0.5, 32 - 0.5, 0 - 0.5, 32 - 0.5);
	renderer.drawLine(32 - 0.5, -32 - 0.5, 0 - 0.5, 32 - 0.5, -32 - 0.5, 32 - 0.5);
	renderer.drawLine(0 - 0.5, -32 - 0.5, 32 - 0.5, 32 - 0.5, -32 - 0.5, 32 - 0.5);

}