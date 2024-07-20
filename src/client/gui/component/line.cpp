
#include "line.hpp"
#include "client/gui/grid/context.hpp"
#include "client/renderer.hpp"

GuiLine::GuiLine(Box2D box, Color color, float weight)
: GuiComponent(box), color(color), weight(weight) {}

void GuiLine::draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) {
	Box2D box = grid.getScreenBox(bounding);

	renderer.setTint(color);
	renderer.setLineSize(weight);
	renderer.drawLine(box.begin(), box.end());

	drawDebugOutline(grid, input, renderer, "GuiLine", 255, 77, 175);
}

bool GuiLine::onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) {
	return false;
}

void GuiLine::navigatorUpdate(GridNavigator& grid) {

}

/*
 * GuiLine Builder
 */

GuiLine::Builder& GuiLine::Builder::tint(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	this->color = Color {r, g, b, a};
	return *this;
}

GuiLine::Builder& GuiLine::Builder::weight(float weight) {
	this->line_weight = weight;
	return *this;
}

GuiLine::Builder& GuiLine::Builder::to(int x, int y) {
	this->tx = x;
	this->ty = y;
	return *this;
}

Box2D GuiLine::Builder::getGridBox() const {
	return {0, 0, tx, ty};
}

Box2D GuiLine::Builder::getBoundBox() const {
	return getGridBox();
}

ComponentProducer GuiLine::Builder::build() const {
	return [box = this->getBoundBox(), color = this->color, weight = this->line_weight] (int x, int y) {
		return new GuiLine {box.offset(x, y), color, weight};
	};
}