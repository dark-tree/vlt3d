
#include "test.hpp"

TestScreen::~TestScreen() = default;

InputResult TestScreen::onKey(ScreenStack& stack, InputContext& input, InputEvent key) {
	if (key.isReleased(GLFW_KEY_SPACE)) {
		logger::info("Toggled test screen visibility");
		test = !test;
		return InputResult::CONSUME;
	}

	if (key.isReleased(GLFW_KEY_ESCAPE)) {
		remove();
		return InputResult::CONSUME;
	}

	return InputResult::PASS;
}

void TestScreen::draw(ImmediateRenderer& renderer, Camera& camera) {

	if (!test) {
		renderer.setTint(255, 255, 0);
		renderer.setFontSize(2);
		renderer.drawText(10, 10, "Press [SPACE] to show");
		renderer.drawText(10, 10 + 9 * 2, "Press [ESCAPE] to close");
		return;
	}

	float t = glfwGetTime() * 1.33;
	float ox = sin(t);
	float oy = cos(t);

	renderer.setFacing(camera);

	renderer.setTint(255, 255, 255);
	renderer.setFontSize(2);
	renderer.setLineSize(4);
	renderer.drawSprite(10, 10, 100, 100, renderer.getSprite("assets/sprites/vkblob.png"));

	renderer.setTint(255, 255, 0);
	renderer.drawText(10, 10, "Press [SPACE] to hide");
	renderer.drawText(10, 10 + 9 * 2, "Press [ESCAPE] to close");

	renderer.setTint(10, 100, 220);
	renderer.drawLine(50, 200, 50, 550);
	renderer.drawLine(50, 550, 150, 650);
	renderer.drawLine(150, 650, 900, 650);
	renderer.drawLine(300, 300, 300 + ox * 150, 300 + oy * 150);

	renderer.setTint(255, 255, 255);
	renderer.setFontSize(0.05);
	renderer.setLineSize(0.05);
	renderer.drawSprite(10 * ox + 10, -3, 10 * oy + 10, 1, 1, renderer.getSprite("assets/sprites/vkblob.png"));
	renderer.drawText(0, 0, 0, "Hello!");
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