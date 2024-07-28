
#include "test.hpp"
#include "pause.hpp"

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

void TestScreen::draw(ImmediateRenderer& renderer, InputContext& input, Camera& camera, bool focused) {

	if (focused) {
		input.setMouseCapture(true);
	}

	if (!test) {
		renderer.setAlignment(HorizontalAlignment::LEFT);
		renderer.setAlignment(VerticalAlignment::TOP);
		renderer.setTint(255, 255, 0);
		renderer.setFontSize(2);
		renderer.drawText(10, 10 + 9 * 2, "Press [SPACE] to show");
		renderer.drawText(10, 10 + 9 * 4, "Press [ESCAPE] to close");
		return;
	}

	float t = glfwGetTime() * 1.33;
	float ox = sin(t);
	float oy = cos(t);

	renderer.setTint(255, 255, 255);
	renderer.drawPatch(renderer.getWidth() - 160 - 32, 32, 10, 10, 16, renderer.getNinePatch("gui-smol", 8));

	renderer.setTint(50, 255, 50);
	renderer.setFontSize(2);

	renderer.setAlignment(HorizontalAlignment::LEFT);
	renderer.setAlignment(VerticalAlignment::TOP);
	renderer.drawText(500, 500, "LEFT TOP");

	renderer.setAlignment(HorizontalAlignment::RIGHT);
	renderer.setAlignment(VerticalAlignment::TOP);
	renderer.drawText(500, 500, "RIGHT TOP");

	renderer.setAlignment(HorizontalAlignment::LEFT);
	renderer.setAlignment(VerticalAlignment::BOTTOM);
	renderer.drawText(500, 500, "LEFT BOTTOM");

	renderer.setAlignment(HorizontalAlignment::RIGHT);
	renderer.setAlignment(VerticalAlignment::BOTTOM);
	renderer.drawText(500, 500, "RIGHT BOTTOM");

	renderer.setFacing(camera);

	renderer.setTint(255, 255, 255);
	renderer.setFontSize(2);
	renderer.setLineSize(4);
	renderer.drawSprite(10, 10, 100, 100, renderer.getSprite("vkblob"));

	renderer.setAlignment(HorizontalAlignment::LEFT);
	renderer.setAlignment(VerticalAlignment::TOP);
	renderer.setTint(255, 255, 0);
	renderer.drawText(10, 10, "FPS: " + std::to_string(profiler.getCountPerSecond()));
	renderer.drawText(10, 10 + 9 * 2, "Press [SPACE] to hide");
	renderer.drawText(10, 10 + 9 * 4, "Press [ESCAPE] to close");

	renderer.setTint(10, 100, 220);
	renderer.drawLine(50, 200, 50, 550);
	renderer.drawLine(50, 550, 150, 650);
	renderer.drawLine(150, 650, 900, 650);
	renderer.drawLine(300, 300, 300 + ox * 150, 300 + oy * 150);

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

	renderer.setTint(255, 0, 0);
	renderer.drawCircle(16, +40, 16, 8);

	renderer.setTint(0, 255, 0);
	renderer.drawCircle(16, -40, 16, 8);

	renderer.setTint(255, 255, 255);
	renderer.drawBar(renderer.getWidth() - 32 - 228, renderer.getHeight() - 64, 228, 32, 1, renderer.getSprite("button"), 4, 4, 0, 32);
	renderer.drawBar(renderer.getWidth() - 32 - 228, renderer.getHeight() - 64, 228, 32, (sin(t * 2) + 1) / 2, renderer.getSprite("button"), 4, 4, 1, 32);


}