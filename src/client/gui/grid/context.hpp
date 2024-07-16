#pragma once

#include "external.hpp"

#include "client/gui/stack.hpp"
#include "util/box.hpp"
#include "window/input.hpp"
#include "window/event.hpp"
#include "client/gui/component/component.hpp"

class GridContext {

	public:

		// dimensions of the grid in cells
		int width;
		int height;

		// screen-space-like attachment point, (0, 0) is the top left corner while (1, 1) is the bottom right one
		float ax;
		float ay;

		// attachment point translated into gui pixels
		float sax;
		float say;

		// the size of one cell, bounding box of the whole grid
		int size;
		Box2D bounding;

		std::shared_ptr<GuiComponent> root;

	public:

		GridContext(int width, int height, int size);
		GridContext(int width, int height, int ox, int oy, float ax, float ay, int size);

		bool shouldAccept(Box2D box, InputContext& input, const InputEvent& event);
		Box2D getScreenBox(Box2D box) const;
		InputResult onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event);
		void draw(ImmediateRenderer& renderer, InputContext& input);

};
