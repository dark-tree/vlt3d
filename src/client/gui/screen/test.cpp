
#include "test.hpp"
#include "pause.hpp"
#include "world/render/renderer.hpp"

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

	float t = glfwGetTime() * 1.33;
	float ox = sin(t);
	float oy = cos(t);

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

	renderer.setFontSize(2);
	renderer.setLineSize(4);
	renderer.setAlignment(HorizontalAlignment::LEFT);
	renderer.setAlignment(VerticalAlignment::TOP);
	renderer.setTint(255, 255, 0);

	auto format = [] (double value, int places) -> std::string {
		std::stringstream stream;
		stream << std::fixed << std::setprecision(places) << value;
		return stream.str();
	};

	int fps = profiler.getCountPerSecond();
	double avg = profiler.getAvgFrameTime();
	double delta = profiler.getMaxFrameTime() - avg;
	int vertices = world_vertex_count;
	int chunks = world_chunk_count;
	int visible = world_visible_count;
	int occluders = world_occlusion_count;

	int width = renderer.getWidth();

	glm::vec3 pos = camera.getPosition();
	renderer.drawText(10, 10, "FPS: " + std::to_string(fps) + " (avg: " + format(avg, 2) + " ms, +" + format(delta, 2) + ") ");
	renderer.drawText(10, 10 + 18 * 1, "X: " + format(pos.x, 4) + ", Y: " + format(pos.y, 4) + ", Z: " + format(pos.z, 4));
	renderer.drawText(10, 10 + 18 * 2, "Vertices: " + std::to_string(vertices) + ", chunks: " + std::to_string(visible) + "/" + std::to_string(chunks));
	renderer.drawText(10, 10 + 18 * 3, "Free Identifiers: " + std::to_string(occluders));

	renderer.setAlignment(HorizontalAlignment::RIGHT);
	renderer.drawText(width - 10, 10 + 18 * 0, test ? "Press [SPACE] to hide" : "Press [SPACE] to show");
	renderer.drawText(width - 10, 10 + 18 * 1, "Press [ESCAPE] to pause");

	if (test) {

		auto history = profiler.getAvgFrameTimeHistory();
		int capacity = decltype(history)::capacity;
		double scale = 350.0 / capacity;
		int head = history.head();

		renderer.setLineSize(0.5);
		renderer.setTint(255, 255, 20, 255);

		for (int i = 0; i < capacity; i++) {
			double offset = scale * i;

			int a = 200 - history.at(i) * 10;
			int b = 200 - history.at(i + 1) * 10;

			if (i == head) b = a;
			if (i == head - 1) a = b;

			renderer.drawLine(offset, a, offset + scale, b);
		}

		renderer.setLineSize(1);
		renderer.setTint(255, 0, 0, 255);
		renderer.drawLine(scale * head, 200, scale * head, 100);

		renderer.setTint(255, 255, 255);
		renderer.setFontSize(0.05);
		renderer.setLineSize(0.05);
		renderer.drawTiled(10 * ox + 10, -3, 10 * oy + 10, 2.6, 2.6, renderer.getSprite("vkblob"), 1, 1);

		renderer.setAlignment(VerticalAlignment::CENTER);
		renderer.setAlignment(HorizontalAlignment::CENTER);
		renderer.drawText(-0.5, -0.5, -0.5, "Hello !");
		renderer.drawLine(0, -3, 0, 10 * ox + 10, -3, 10 * oy + 10);

	}

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

	if (test) {
		renderer.setTint(255, 255, 255);
		renderer.drawBar(renderer.getWidth() - 32 - 228, renderer.getHeight() - 64, 228, 32, 1, renderer.getSprite("button"), 4, 4, 0, 32);
		renderer.drawBar(renderer.getWidth() - 32 - 228, renderer.getHeight() - 64, 228, 32, (sin(t * 2) + 1) / 2, renderer.getSprite("button"), 4, 4, 1, 32);
	}

}