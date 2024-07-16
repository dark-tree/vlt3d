#pragma once

#include "external.hpp"
#include "component.hpp"
#include "util/exception.hpp"

class GuiComposed : public GuiComponent {

	private:

		std::vector<std::shared_ptr<GuiComponent>> components;

	public:

		GuiComposed(Box2D box, const std::vector<std::shared_ptr<GuiComponent>>& components);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) override;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) override;

	public:

		class Builder : public ComponentBuilder {

			protected:

				Box2D combined;
				int x = 0, y = 0;

				std::vector<ComponentFactory> children;

			private:

				const ComponentFactory& getLast();
				void append(ComponentFactory factory);

			public:

				Builder& add(int x, int y);
				Builder& add(int x, int y, ComponentBuilder& builder);
				Builder& then(Chain chain, ComponentBuilder& builder);

				Box2D getGridBox() const final;
				Box2D getBoundBox() const final;
				ComponentProducer build() const final;

		};

		static inline GuiComposed::Builder of() {
			GuiComposed::Builder builder;
			return builder;
		}

};
