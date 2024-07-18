#pragma once

#include "external.hpp"
#include "component.hpp"

class GuiImage : public GuiComponent {

	private:

		std::string identifier;
		Color color;

	public:

		GuiImage(Box2D box, const std::string& identifier, Color color);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) final;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) final;
		virtual void navigatorUpdate(GridNavigator& grid) final;

		class Builder : public BoundComponentBuilder<GuiImage::Builder> {

			protected:

				std::string identifier;
				Color color;

			public:

				/// Sets the sprite to display
				Builder& sprite(const std::string& identifier);

				/// Sets the tint to use on the image
				Builder& tint(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

			public:

				ComponentProducer build() const final;

		};

		static inline GuiImage::Builder of() {
			GuiImage::Builder builder;
			return builder;
		}

};
