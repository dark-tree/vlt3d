#pragma once

#include "external.hpp"
#include "component.hpp"
#include "util/color.hpp"

class GuiImage : public GuiComponent {

	private:

		std::string identifier;
		Color color;

	public:

		GuiImage(Box2D box, const std::string& identifier, Color color);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) override;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) override;

		class Builder : public BoundComponentBuilder<GuiImage::Builder> {

			protected:

				std::string identifier;
				Color color;

			public:

				Builder& sprite(const std::string& identifier);
				Builder& tint(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

				ComponentProducer build() const final;

		};

		static inline GuiImage::Builder of() {
			GuiImage::Builder builder;
			return builder;
		}

};
