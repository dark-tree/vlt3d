#pragma once

#include "external.hpp"
#include "component.hpp"
#include "util/color.hpp"

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

			public:

				Builder& tint(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
				Builder& weight(float weight);
				Builder& to(int x, int y);

				Box2D getGridBox() const final;
				Box2D getBoundBox() const final;

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
