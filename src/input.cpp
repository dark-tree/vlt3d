
#include "input.hpp"

int window_width, window_height;
int mouse_x, mouse_y;
std::list<int> keys;

bool Input::open(int width, int height, const char* title) {
	winxHint(WINX_HINT_OPENGL_MAJOR, 4);
	winxHint(WINX_HINT_OPENGL_MINOR, 3);
	winxHint(WINX_HINT_OPENGL_CORE, true);

	if (!winxOpen(width, height, title)) {
		logger::fatal("Error occured during window creation! ", winxGetError());
		return false;
	}

	// winx does not guarantee to call the resize callback
	// on window ceration
	window_width = width;
	window_height = height;

	// capture and hide cursor
	winxSetCursorCapture(true);
	winxSetCursorIcon(winxCreateNullCursorIcon());

	winxSetResizeEventHandle([] (int width, int height) {
		glViewport(0, 0, width, height);

		window_width = width;
		window_height = height;
	});

	winxSetKeybordEventHandle([] (int state, int keycode){
		if (state == WINX_PRESSED) {
			keys.push_back(keycode);
		} else {
			keys.remove(keycode);
		}
	});

	winxSetCursorEventHandle([] (int x, int y) {
		const int xr = window_width / 2;
		const int yr = window_height / 2;

		if ((xr != x || yr != y) && winxGetFocus()) {
			mouse_x += (x - xr);
			mouse_y += (y - yr);
			winxSetCursorPos(xr, yr);
		}
	});

	return true;
}

bool Input::isPressed(int keycode) {
	return std::find(keys.begin(), keys.end(), keycode) != keys.end();
}

void Input::getMousePos(int& x, int& y) {
	x = mouse_x;
	y = mouse_y;
}

