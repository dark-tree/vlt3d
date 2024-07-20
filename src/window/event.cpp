
#include "event.hpp"

KeycodeInputEvent::KeycodeInputEvent(int identifier, int mods, int action) {
	this->identifier = identifier;
	this->mods = mods;
	this->action = action;
}

bool KeycodeInputEvent::isPressed() const {
	return action == GLFW_PRESS;
}

bool KeycodeInputEvent::isReleased() const {
	return action == GLFW_RELEASE;
}

bool KeycodeInputEvent::isCtrlHeld() const {
	return mods & GLFW_MOD_CONTROL;
}

bool KeycodeInputEvent::isAltHeld() const {
	return mods & GLFW_MOD_ALT;
}

bool KeycodeInputEvent::isShiftHeld() const {
	return mods & GLFW_MOD_SHIFT;
}

MouseEvent::MouseEvent(int button, int mods, int action)
: KeycodeInputEvent(button, mods, action) {}

bool MouseEvent::isLeft() const {
	return identifier == GLFW_MOUSE_BUTTON_LEFT;
}

bool MouseEvent::isRight() const {
	return identifier == GLFW_MOUSE_BUTTON_RIGHT;
}

bool MouseEvent::hasLeftClicked() const {
	return isLeft() && isReleased();
}

bool MouseEvent::hasRightClicked() const {
	return isRight() && isReleased();
}

InputEvent::Type MouseEvent::getType() const {
	return MouseEvent::self_type;
}

KeyboardEvent::KeyboardEvent(int key, int mods, int action)
: KeycodeInputEvent(key, mods, action) {}

bool KeyboardEvent::isKeyPressed(int key) const {
	return isPressed() && (identifier == key);
}

bool KeyboardEvent::isKeyReleased(int key) const {
	return isReleased() && (identifier == key);
}

InputEvent::Type KeyboardEvent::getType() const {
	return KeyboardEvent::self_type;
}

ScrollEvent::ScrollEvent(float scroll)
: scroll(scroll) {}

float ScrollEvent::getScroll() const {
	return scroll;
}

InputEvent::Type ScrollEvent::getType() const {
	return ScrollEvent::self_type;
}
