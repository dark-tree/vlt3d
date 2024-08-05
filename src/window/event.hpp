#pragma once

#include "external.hpp"

class InputEvent {

	public:

		enum Type {
			KEYBOARD,
			BUTTON,
			SCROLL
		};

	public:

		virtual Type getType() const = 0;

		template <typename T> requires requires { { T::self_type } -> std::convertible_to<Type>; }
		const T* as() const {
			return getType() == T::self_type ? static_cast<const T*>(this) : nullptr;
		}

};

class KeycodeInputEvent : public InputEvent {

	protected:

		READONLY int identifier;
		READONLY int mods;
		READONLY int action;

	public:

		KeycodeInputEvent(int identifier, int mods, int action);

		bool isPressed() const;
		bool isReleased() const;

		bool isCtrlHeld() const;
		bool isAltHeld() const;
		bool isShiftHeld() const;

};

class ButtonEvent : public KeycodeInputEvent {

	public:

		static constexpr Type self_type = InputEvent::BUTTON;

	public:

		ButtonEvent(int button, int mods, int action);

		bool isLeft() const;
		bool isRight() const;
		bool hasLeftClicked() const;
		bool hasRightClicked() const;

		InputEvent::Type getType() const override;

};

class KeyboardEvent : public KeycodeInputEvent {

	public:

		static constexpr Type self_type = InputEvent::KEYBOARD;

	public:

		KeyboardEvent(int key, int mods, int action);

		bool isKeyPressed(int key) const;
		bool isKeyReleased(int key) const;

		InputEvent::Type getType() const override;



};

class ScrollEvent : public InputEvent {

	public:

		static constexpr Type self_type = InputEvent::SCROLL;

	private:

		READONLY float scroll;

	public:

		ScrollEvent(float scroll);

		float getScroll() const;

		InputEvent::Type getType() const override;

};



