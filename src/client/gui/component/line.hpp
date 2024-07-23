#pragma once

#include "external.hpp"
#include "component.hpp"

class GuiLine : public GuiComponent {

	private:

		Color color;
		float weight;

	public:

		GuiLine(Box2D box, Color color, float weight);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) final;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) final;
		virtual void navigatorUpdate(GridNavigator& grid) final;

		class Builder : public ComponentBuilder {

			private:

				Color color;
				float line_weight;
				float tx, ty;

				Box2D getGridBox() const final;
				Box2D getBoundBox() const final;

			public:

				/// Sets the color of the line
				Builder& tint(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

				/// Sets thickness of the line in pixels
				Builder& weight(float weight);

				/// Sets the relative end of the line
				Builder& to(int x, int y);

			public:

				ComponentProducer build() const final;

		};

		static inline GuiLine::Builder of() {
			GuiLine::Builder builder;
			builder.to(0, 0);
			builder.weight(2);
			builder.tint(0, 0, 0);
			return builder;
		}

};
