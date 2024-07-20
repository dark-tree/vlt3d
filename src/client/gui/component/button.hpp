#pragma once

#include "external.hpp"
#include "component.hpp"

class GuiButton : public GuiComponent {

	private:

		std::string text;
		std::string identifier;
		std::function<void(ScreenStack&)> callback;

	public:

		GuiButton(Box2D box, std::string text, std::string identifier, std::function<void(ScreenStack&)> callback);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) final;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) final;
		virtual void navigatorUpdate(GridNavigator& grid) final;

		class Builder : public BoundComponentBuilder<GuiButton::Builder> {

			private:

				std::string string;
				std::string identifier;
				std::function<void(ScreenStack&)> callback;

			public:

				/// Sets the text to be displayed over the button
				Builder& text(const std::string& text);

				/// Sets the sprite to use for the button background
				Builder& sprite(const std::string& identifier);

				/// Sets the function to invoke when the button is pressed
				Builder& then(std::function<void(ScreenStack&)> callback);

			public:

				ComponentProducer build() const final;

		};

		static inline GuiButton::Builder of() {
			GuiButton::Builder builder;
			return builder;
		}

};