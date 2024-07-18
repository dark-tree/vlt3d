
#include "spacer.hpp"

GuiSpacer::GuiSpacer(Box2D box)
: GuiComponent(box) {}

void GuiSpacer::draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) {
	drawDebugOutline(grid, input, renderer, "GuiSpacer", 80, 80, 80, 0.1);
}

bool GuiSpacer::onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) {
	return false;
}

/*
 * GuiSpacer Builder
 */

ComponentProducer GuiSpacer::Builder::build() const {
	return [box = this->getBoundBox()] (int x, int y) {
		return new GuiSpacer {box.offset(x, y)};
	};
}