#pragma once

#include "external.hpp"
#include "component.hpp"

class GuiImage : public GuiComponent {

	private:

		READONLY std::string identifier;

	public:

		GuiImage(Box2D box, const std::string& identifier);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) override;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) override;

		class Builder : public BoundComponentBuilder<GuiImage::Builder> {

			protected:

				std::string identifier;

			public:

				Builder& sprite(const std::string& identifier);

				ComponentProducer build() const final;

		};

		static inline GuiImage::Builder of() {
			return {};
		}

};
