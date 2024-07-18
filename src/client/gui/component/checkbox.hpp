#pragma once

#include "external.hpp"
#include "component.hpp"

class GuiCheck : public GuiComponent {

	private:

		std::string label;
		std::function<void(bool)> callback;
		bool state;

	public:

		GuiCheck(Box2D box, std::string label, std::function<void(bool)> callback, bool initial);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) final;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) final;
		virtual void navigatorUpdate(GridNavigator& grid) final;

		class Builder : public BoundComponentBuilder<GuiCheck::Builder> {

			private:

				bool value;
				std::string string;
				std::function<void(bool)> callback;

			public:

				/// Sets the label of the check box, that is, the text to its right
				Builder& label(const std::string& text);

				/// Sets the function that should be called when the checkbox is switched
				Builder& then(std::function<void(bool)> callback);

				/// Sets the initial value of the checkbox
				Builder& initial(bool initial);

			public:

				ComponentProducer build() const final;

		};

		static inline GuiCheck::Builder of() {
			GuiCheck::Builder builder;
			builder.then([] (bool state) {});
			builder.initial(false);
			return builder;
		}

};