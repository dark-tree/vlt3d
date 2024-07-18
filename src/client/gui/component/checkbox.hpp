#pragma once

#include "external.hpp"
#include "component.hpp"
#include "client/renderer.hpp"

class GuiCheck : public GuiComponent {

	private:

		bool state;
		std::string label;
		std::function<void(bool)> callback;

	public:

		GuiCheck(Box2D box, std::string label, std::function<void(bool)> callback, bool initial);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) final;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) final;
		virtual void navigatorUpdate(GridNavigator& grid) final;

		class Builder : public BoundComponentBuilder<GuiCheck::Builder> {

			private:

				std::string string;
				std::function<void(bool)> callback;

			public:

				Builder& label(const std::string& text);
				Builder& then(std::function<void(bool)> callback);

				ComponentProducer build() const final;

		};

		static inline GuiCheck::Builder of() {
			GuiCheck::Builder builder;
			return builder;
		}

};