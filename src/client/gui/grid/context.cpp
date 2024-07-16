
#include "context.hpp"

bool GridContext::isDebugMode() const {
	return true;
}

bool GridContext::shouldAccept(Box2D box, InputContext& input, const InputEvent& event) {
	if (event.getType() == InputEvent::KEYBOARD) {
		return true;
	}

	if (event.getType() == InputEvent::MOUSE) {
		return input.isMouseWithin(box.offset(sax, say));
	}

	return false;
}

GridContext::GridContext(int width, int height, int size)
: GridContext(width, height, - width * size / 2, - height * size / 2, 0.5f, 0.5f, size) {}

GridContext::GridContext(int width, int height, int ox, int oy, float ax, float ay, int size)
: width(width), height(height), ax(ax), ay(ay), size(size), bounding(Box2D (0, 0, width, height).scale(size).offset(ox, oy)) {
	this->sax = -1;
	this->say = -1;
}

Box2D GridContext::getScreenBox(Box2D box) const {
	return box.scale(size).offset(sax, say).offset(bounding.begin());
}

InputResult GridContext::onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) {
	if (root != nullptr && shouldAccept(bounding, input, event)) {
		return root->onEvent(*this, stack, input, event) ? InputResult::CONSUME : InputResult::BLOCK;
	}

	return InputResult::PASS;
}

void GridContext::draw(ImmediateRenderer& renderer, InputContext& input) {
	this->sax = renderer.getWidth() * ax;
	this->say = renderer.getHeight() * ay;

	// screen adjusted box
	Box2D box = bounding.offset(sax, say);

	// TODO draw (sax, say)

	if (input.isMouseWithin(box)) {
		renderer.setTint(155, 255, 155);
	} else {
		renderer.setTint(255, 155, 155);
	}

	renderer.drawSprite(box.begin(), box.width(), box.height(), renderer.getSprite("assets/sprites/blank.png"));

	renderer.setLineSize(1);
	renderer.setTint(80, 110, 245);

	// draw vertical grid lines
	for (int x = 1; x < width; x ++) {
		const float vx = box.x1 + x * box.width() / width;
		renderer.drawLine(vx, box.y1, vx, box.y2);
	}

	// draw horizontal grid lines
	for (int y = 1; y < height; y ++) {
		const float vy = box.y1 + y * box.height() / height;
		renderer.drawLine(box.x1, vy, box.x2, vy);
	}

	if (root != nullptr) {
		root->draw(*this, input, renderer);
	}
}