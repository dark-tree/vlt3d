#pragma once

#include "external.hpp"
#include "util/box.hpp"
#include "util/color.hpp"
#include "client/alignment.hpp"
#include "client/gui/grid/inset.hpp"
#include "client/gui/grid/chain.hpp"

class GridNavigator;
class GridContext;
class ScreenStack;
class InputContext;
class InputEvent;
class ImmediateRenderer;

/**
 * Factory for creating instances of configured components, each call provides
 * an offset in grids as arguments (the two ints x & y) this is the grid placement position
 * of the newly constructed component, so the final grid bounding box of the component is
 * equal to `getBoundBox().offset(x, y)`, Note: one need to copy the box before offseting it
 *
 * @verbatim
 * + ---------------- +            + ----------------- +           + ---------- +
 * | ComponentBuilder | -build()-> | ComponentProducer | -(x, y)-> | Component* |
 * + ---------------- +            + ----------------- +           + ---------- +
 *                                           |                            ^
 *                                         (x, y)   See: GuiComposed    (x, y)
 *                                           |    + ---------------- +    |
 *                                           \__> | ComponentFactory | ___/
 *                                                + ---------------- +
 */
using ComponentProducer = std::function<class GuiComponent*(int x, int y)>;

/**
 * All component builders must implement this interface
 * for most simple components looks at the BoundComponentBuilder
 * that already implements the `getGridBox()` and `getBoundBox()` methods
 *
 * @note
 * When building a GUI never invoke any of those methods yourself!
 */
class ComponentBuilder {

	public:

		virtual ~ComponentBuilder() = default;

		/**
		 * Returns a relative box in grid coordinates that contains the component
		 * without taking into account insets/outsets
		 */
		virtual Box2D getGridBox() const = 0;

		/**
		 * Returns a relative box in grid coordinates that contains the component
		 * this method differs from `getGridBox()` by including insets/outsets
		 */
		virtual Box2D getBoundBox() const = 0;

		/**
		 * Bakes this builder into a `ComponentProducer` factory,
		 * all builder states MUST be copied no referenced!
		 */
		virtual ComponentProducer build() const = 0;

};

/**
 * Specification of the ComponentBuilder interface that implements the box
 * configuration methods, this is what should be used as a based for builder of most components
 * except ones that change their size based on more complex rules (like for example `GuiComposed`).
 *
 * @note
 * Supply the type of the inheriting builder as template parameter `T`, so that
 * methods in this class can returns the correct child type.
 */
template <class T>
struct BoundComponentBuilder : public ComponentBuilder {

	protected:

		int width = 1;
		int height = 1;
		Inset insets;

		Box2D getGridBox() const final {
			return {0, 0, (float) width, (float) height};
		}

		Box2D getBoundBox() const final {
			return {insets.left, insets.bottom, width - insets.right, height - insets.top};
		}

	public:

		/// Sets the dimensions of the component, as a width and height in grids
		T& box(int w, int h) {
			this->width = w;
			this->height = h;
			return dynamic_cast<T&>(*this);
		}

		/// Sets an equal margin around the component as a fraction of a grid
		T& inset(float margin) {
			return inset(margin, margin);
		}

		/// Sets a margin around the component in vertical and horizontal directions as a fraction of a grid
		T& inset(float vertical, float horizontal) {
			return inset(vertical, vertical, horizontal, horizontal);
		}

		/// Sets a margin around the component for all directions as a fraction of a grid
		T& inset(float top, float bottom, float right, float left) {
			insets.top = top;
			insets.bottom = bottom;
			insets.right = right;
			insets.left = left;
			return dynamic_cast<T&>(*this);
		}

};

class GuiComponent {

	protected:

		READONLY Box2D bounding;

	protected:

		GuiComponent(Box2D box);

		/**
		 * Utility function for rendering debugging information when in debug mode
		 * call it at the end of the draw() method
		 */
		void drawDebugOutline(GridContext& grid, InputContext& input, ImmediateRenderer& renderer, const char* name, uint8_t r, uint8_t g, uint8_t b, float outset = 0) const;

	public:

		virtual ~GuiComponent() = default;

		/**
		 * Called every frame on the component to render itself
		 * this method must be thread safe, as it can be called from multiple threads
		 */
		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) = 0;

		/**
		 * Called from the main thread thread for each incoming event implementors should use
		 * the `GridContext::shouldAccept` to check if the event should be processed
		 *
		 * @note
		 * Key press events for the 'Tab' and 'End' keys will not ever reach this method
		 * as they will be consumed by the grid's navigator system
		 */
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) = 0;

		/**
		 * Called when the underlying grid context changes or is provided for the first time
		 * use this method to append interactable components to the navigator using @code grid.addNavigatorNode(this) @endcode
		 */
		virtual void navigatorUpdate(GridNavigator& grid) = 0;

};