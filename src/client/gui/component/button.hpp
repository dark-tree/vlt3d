#pragma once

#include "external.hpp"
#include "component.hpp"
#include "client/renderer.hpp"

class GuiButton : public GuiComponent {

	private:

		std::string text;
		std::string identifier;
		std::function<void(ScreenStack&)> callback;

	public:

		GuiButton(Box2D box, std::string  text, std::string  identifier, std::function<void(ScreenStack&)> callback);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) override;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) override;

		class Builder : public BoundComponentBuilder<GuiButton::Builder> {

			private:

				std::string string;
				std::string identifier;
				std::function<void(ScreenStack&)> callback;

			public:

				Builder& text(const std::string& text);
				Builder& sprite(const std::string& identifier);
				Builder& then(std::function<void(ScreenStack&)> callback);

				ComponentProducer build() const final;

		};

		static inline GuiButton::Builder of() {
			GuiButton::Builder builder;
			return builder;
		}

};