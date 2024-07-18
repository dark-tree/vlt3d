#pragma once

#include "external.hpp"
#include "component.hpp"
#include "client/renderer.hpp"

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

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) override;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) override;

		class Builder : public BoundComponentBuilder<GuiText::Builder> {

			private:

				Color color;
				float font_size, font_tilt;
				std::string string;
				VerticalAlignment vertical;
				HorizontalAlignment horizontal;

			public:

				Builder& tint(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
				Builder& text(const std::string& text);
				Builder& size(float font_size);
				Builder& italics(float font_tilt = 1);
				Builder& align(VerticalAlignment alignment);
				Builder& align(HorizontalAlignment alignment);
				Builder& align(VerticalAlignment vertical, HorizontalAlignment alignment);

				/**
				 * Shorthand for calling
				 * @code .align(VerticalAlignment::CENTER)
				 * .align(HorizontalAlignment::CENTER)
				 */
				Builder& center();

				/**
				 * Shorthand for calling
				 * @code .align(VerticalAlignment::CENTER)
				 * .align(HorizontalAlignment::LEFT)
				 */
				Builder& left();

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