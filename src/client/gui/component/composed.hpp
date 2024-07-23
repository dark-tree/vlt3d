#pragma once

#include "external.hpp"
#include "component.hpp"

/**
 * A thin wrapper over the `ComponentProducer` `std::function`,
 * used internally by the GuiComposed to retain some extra information about the
 * component being appended without needing to keep a direct
 * reference to the templated builder class as well as provide internal offset.
 * <p>
 * This internal offset allows the component to be doubly-offseted (first within
 * `GuiComposed` then by the offset of the `GuiComposed` itself, this is needed to
 * ensure the GUI structure can be nested)
 */
struct ComponentFactory {

	private:

		int ox, oy;
		Box2D grid, bounding;
		ComponentProducer producer;

	public:

		ComponentFactory(int x, int y, ComponentBuilder& builder);

		/// Returns a copy of `ComponentBuilder::getGridBox()`
		Box2D getGridBox() const;

		/// Returns a copy of `ComponentBuilder::getBoundBox()`
		Box2D getBoundBox() const;

		/// Invokes the stored `ComponentProducer` with the specified offset plus the offset given as an argument to the constructor
		GuiComponent* build(int x, int y) const;

};

/**
 * An integral GUI Component for the grid system,
 * allows grouping of multiple components into one
 */
class GuiComposed : public GuiComponent {

	private:

		std::vector<std::shared_ptr<GuiComponent>> components;

	public:

		GuiComposed(Box2D box, const std::vector<std::shared_ptr<GuiComponent>>& components);

	public:

		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) final;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) final;
		virtual void navigatorUpdate(GridNavigator& grid) final;

	public:

		class Builder : public ComponentBuilder {

			private:

				Box2D combined;
				int x = 0, y = 0;
				std::vector<ComponentFactory> children;

				Box2D getGridBox() const final;
				Box2D getBoundBox() const final;

				const ComponentFactory& getLast();
				void append(ComponentFactory factory);

			public:

				/// Adds a dummy spacer element at the given offset
				Builder& add(int x, int y);

				/// Adds the specified component at the given offset
				Builder& add(int x, int y, ComponentBuilder& builder);

				/// Adds the specified component in some relation to the previous one
				Builder& then(Chain chain, ComponentBuilder& builder);

			public:

				ComponentProducer build() const final;

		};

		static inline GuiComposed::Builder of() {
			GuiComposed::Builder builder;
			return builder;
		}

};
