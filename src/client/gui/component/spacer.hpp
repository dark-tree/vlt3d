#pragma once

#include "external.hpp"
#include "component.hpp"

class GuiSpacer : public GuiComponent {

	public:

		GuiSpacer(Box2D box);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) final;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) final;
		virtual void navigatorUpdate(GridNavigator& grid) final;

		class Builder : public BoundComponentBuilder<GuiSpacer::Builder> {

			public:

				ComponentProducer build() const final;

		};

		static inline GuiSpacer::Builder of() {
			GuiSpacer::Builder builder;
			builder.box(0, 0);
			return builder;
		}

};
