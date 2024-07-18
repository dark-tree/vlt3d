#pragma once

#include "external.hpp"
#include "component.hpp"

class GuiText : public GuiComponent {

	private:

		Color color;
		float size, tilt;
		std::string text;
		VerticalAlignment vertical;
		HorizontalAlignment horizontal;

	public:

		GuiText(Box2D box, Color color, float size, float tilt, const std::string& text, VerticalAlignment vertical, HorizontalAlignment horizontal);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) final;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) final;
		virtual void navigatorUpdate(GridNavigator& grid) final;

		class Builder : public BoundComponentBuilder<GuiText::Builder> {

			private:

				Color color;
				float font_size, font_tilt;
				std::string string;
				VerticalAlignment vertical;
				HorizontalAlignment horizontal;

			public:

				/// Sets the color of the text
				Builder& tint(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

				/// Sets the text to display
				Builder& text(const std::string& text);

				/// Sets the size (in pixels) of the text
				Builder& size(float font_size);

				/// Sets the tilt of the text, use 0 for normal text and 1 for italics
				Builder& italics(float font_tilt = 1);

				/// Sets the vertical alignment (along the Y axis)
				Builder& align(VerticalAlignment alignment);

				/// Sets the horizontal alignment (along the X axis)
				Builder& align(HorizontalAlignment alignment);

				/// Shorthand for calling @code align(VerticalAlignment).align(HorizontalAlignment)
				Builder& align(VerticalAlignment vertical, HorizontalAlignment alignment);

				/// Shorthand for calling @code align(VerticalAlignment::CENTER, HorizontalAlignment::CENTER)
				Builder& center();

				/// Shorthand for calling @code align(VerticalAlignment::CENTER, HorizontalAlignment::LEFT)
				Builder& left();

			public:

				ComponentProducer build() const final;

		};

		static inline GuiText::Builder of() {
			GuiText::Builder builder;
			builder.align(VerticalAlignment::BOTTOM);
			builder.align(HorizontalAlignment::LEFT);
			builder.size(2);
			builder.italics(0);
			return builder;
		}

};