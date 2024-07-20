
#include "context.hpp"

bool GridContext::shouldAccept(Box2D box, InputContext& input, const InputEvent& event) {
	if (event.getType() == InputEvent::KEYBOARD) {
		return true;
	}

	if (event.getType() == InputEvent::MOUSE) {
		return input.isMouseWithin(box);
	}

	return false;
}

GridContext::GridContext(int width, int height, int size)
: GridContext(width, height, - width * size / 2, - height * size / 2, 0.5f, 0.5f, size) {}

GridContext::GridContext(int width, int height, int ox, int oy, float ax, float ay, int size)
: width(width), height(height), size(size), ax(ax), ay(ay), bounding(Box2D (0, 0, width, height).scale(size).offset(ox, oy)) {
	this->sax = -1;
	this->say = -1;
}

Box2D GridContext::getScreenBox(Box2D box) const {
	return box.scale(size).offset(sax, say).offset(bounding.begin());
}

bool GridContext::isDebugMode(InputContext& input) const {
	return input.isKeyPressed(GLFW_KEY_F1);
}

void GridContext::setModel(ComponentProducer model) {
	// reset the navigator
	reset();

	// swap root and scan
	root.reset(model(0, 0));
	root->navigatorUpdate(*this);
}

InputResult GridContext::onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) {
	if (auto* key = event.as<KeyboardEvent>()) {
		if (key->isKeyPressed(GLFW_KEY_TAB)) {
			next();
			return InputResult::CONSUME;
		}

		if (key->isKeyPressed(GLFW_KEY_END)) {
			stop();
			return InputResult::CONSUME;
		}
	}

	if (root != nullptr && GridContext::shouldAccept(getScreenBox(bounding), input, event)) {
		return root->onEvent(*this, stack, input, event) ? InputResult::CONSUME : InputResult::BLOCK;
	}

	return InputResult::PASS;
}

void GridContext::draw(ImmediateRenderer& renderer, InputContext& input) {
	this->sax = renderer.getWidth() * ax;
	this->say = renderer.getHeight() * ay;

	// screen adjusted box
	Box2D box = bounding.offset(sax, say);

	renderer.setTint(255, 255, 255);
	renderer.drawPatch(box.begin(), width, height, size, renderer.getNinePatch("assets/sprites/gui-smol.png", 8));

	if (isDebugMode(input)) {
		renderer.setLineSize(1);
		renderer.setTint(80, 110, 245);

		// draw vertical grid lines
		for (int x = 1; x < width; x++) {
			const float vx = box.x1 + x * box.width() / width;
			renderer.drawLine(vx, box.y1, vx, box.y2);
		}

		// draw horizontal grid lines
		for (int y = 1; y < height; y++) {
			const float vy = box.y1 + y * box.height() / height;
			renderer.drawLine(box.x1, vy, box.x2, vy);
		}

		// show the attachment point
		renderer.setTint(0, 0, 0);
		renderer.drawCircle(sax, say, 12);
		renderer.setTint(0, 255, 0);
		renderer.drawCircle(sax, say, 8);
	}

	if (root != nullptr) {
		root->draw(*this, input, renderer);
	}
}