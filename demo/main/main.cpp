
#include <glad/glad.h>
#include <winx/winx.h>
#include <stdio.h>

#include "render.hpp"
#include "window.hpp"

void closeEventHandle() {
	winxClose();
	exit(0);
}

void resizeEventHandle(int w, int h) {
	glViewport(0, 0, w, h);
}

int main() {

	glphInit(400, 300, "main");

	// init OpenGL example
	init();

	winxSetCloseEventHandle(closeEventHandle);
	winxSetResizeEventHandle(resizeEventHandle);

	while(1) {
		draw();

		winxSwapBuffers();
		winxPollEvents();
	}

}

