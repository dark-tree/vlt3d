#pragma once

#include "external.hpp"

#include "client/gui/stack.hpp"
#include "util/box.hpp"
#include "window/input.hpp"
#include "window/event.hpp"
#include "client/gui/component/component.hpp"
#include "navigator.hpp"

class GridContext : public GridNavigator {

	public:

		// dimensions of the grid in cells and the
		// size of one grid cell in pixels
		int width, height, size;

		// screen-space-like attachment point
		// (0, 0) is the top left corner while (1, 1) is the bottom right one
		float ax, ay;

		// attachment point translated into gui pixels
		// shown as a green circle while in debug mode
		float sax, say;

		Box2D bounding;
		std::shared_ptr<GuiComponent> root;

	public:

		GridContext(int width, int height, int size);
		GridContext(int width, int height, int ox, int oy, float ax, float ay, int size);

		/**
		 * Checks if the given event should be accepted by the
		 * component based on its bounding box in screen space
		 */
		static bool shouldAccept(Box2D box, InputContext& input, const InputEvent& event);

		/**
		 * Translates a given grid bounding box into screen space
		 * based on the current attachment point of the grid and screen dimensions
		 */
		Box2D getScreenBox(Box2D box) const;

		/**
		 * Check is the Grid Debug Mode is enabled and if additional info should be rendered
		 * over the user interface
		 */
		bool isDebugMode() const;

		/**
		 * Changes the model attached to this grid, this method takes care
		 * off all required the initialization, like performing a navigator scan
		 */
		 void setModel(ComponentProducer model);

	public:

		InputResult onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event);
		void draw(ImmediateRenderer& renderer, InputContext& input);

};
