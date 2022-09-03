
#include "window.hpp"

#include <glad/glad.h>
#include <winx/winx.h>
#include <stdio.h>

void glphInit(int w, int h, const char* title) {
	if (!winxOpen(w, h, title)) {
		printf("Error occured! %s\n", winxGetError());
		exit(1);
	}

	// use GLAD to load OpenGL functions
	gladLoadGL();

}

