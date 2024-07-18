#pragma once

#include "external.hpp"
#include "event.hpp"
#include "util/box.hpp"

class Window;

enum struct InputResult {
	PASS,
	CONSUME,
	BLOCK
};

class InputContext {

	private:

		Window& window;

	public:

		InputContext(Window& window);

		glm::vec2 getMouse();
		bool isMouseWithin(Box2D);
		bool isKeyPressed(int key);
		bool isButtonPressed(int button);
		bool isLeftPressed();
		bool isRightPressed();

};

class InputConsumer {

	public:

		virtual ~InputConsumer() = default;

		virtual InputResult onEvent(InputContext& context, const InputEvent& key);

};
