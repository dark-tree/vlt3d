#pragma once

#include "external.hpp"
#include "event.hpp"

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
		bool isKeyPressed(int key);
		bool isButtonPressed(int button);

};

class InputConsumer {

	public:

		virtual ~InputConsumer() = default;

		virtual InputResult onKey(InputContext& context, InputEvent key);
		virtual InputResult onMouse(InputContext& context, InputEvent button);
		virtual InputResult onScroll(InputContext& context, float scroll);


};
