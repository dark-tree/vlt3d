#pragma once

#include "external.hpp"

class InputEvent {

	private:

		READONLY int identifier;
		READONLY int mods;
		READONLY int action;

	public:

		InputEvent() = default;
		InputEvent(int identifier, int mods, int action);

		bool is(int identifier) const;
		bool pressed() const;
		bool released() const;

		bool isPressed(int identifier) const;
		bool isReleased(int identifier) const;
		bool isCtrlHeld() const;
		bool isAltHeld() const;
		bool isShiftHeld() const;

};



