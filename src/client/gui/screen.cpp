
#include "screen.hpp"

Screen::Screen() {
	this->removed = false;
}

void Screen::remove() {
	this->removed = true;
}

void Screen::draw(ImmediateRenderer& renderer, Camera& camera) {
	// by default screen draws nothing, override to implement custom behavior
}