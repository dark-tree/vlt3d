
#include "event.hpp"

InputEvent::InputEvent(int identifier, int mods, int action) {
	this->identifier = identifier;
	this->mods = mods;
	this->action = action;
}

bool InputEvent::is(int identifier) const {
	return this->identifier == identifier;
}

bool InputEvent::pressed() const {
	return this->action == GLFW_PRESS;
}

bool InputEvent::released() const {
	return this->action == GLFW_RELEASE;
}

bool InputEvent::isPressed(int identifier) const {
	return is(identifier) && pressed();
}

bool InputEvent::isReleased(int identifier) const {
	return is(identifier) && released();
}

bool InputEvent::isCtrlHeld() const {
	return mods & GLFW_MOD_CONTROL;
}

bool InputEvent::isAltHeld() const {
	return mods & GLFW_MOD_ALT;
}

bool InputEvent::isShiftHeld() const {
	return mods & GLFW_MOD_SHIFT;
}
