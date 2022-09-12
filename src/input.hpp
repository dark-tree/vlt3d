#pragma once

#include <core.hpp>

namespace Input {

	bool open(int width, int height, const char* title);

	bool isPressed(int keycode);
	void getMousePos(int& x, int& y);

}
