
#include "component.hpp"
#include "client/gui/grid/context.hpp"

GuiComponent::GuiComponent(Box2D box)
: bounding(box) {}

void GuiComponent::drawDebugOutline(GridContext& grid, InputContext& input, ImmediateRenderer& renderer, const char* name, uint8_t r, uint8_t g, uint8_t b, float outset) const {
	if (grid.isDebugMode()) {
		Box2D screen = grid.getScreenBox(this->bounding.inset(-outset));

		renderer.setTint(r, g, b, 255);
		renderer.setLineSize(1);
		renderer.drawLine(screen.x1, screen.y1, screen.x2, screen.y1);
		renderer.drawLine(screen.x1, screen.y1, screen.x1, screen.y2);
		renderer.drawLine(screen.x2, screen.y1, screen.x2, screen.y2);
		renderer.drawLine(screen.x1, screen.y2, screen.x2, screen.y2);

		if (input.isMouseWithin(screen)) {
			renderer.setAlignment(VerticalAlignment::BOTTOM);
			renderer.setAlignment(HorizontalAlignment::LEFT);
			renderer.drawText(screen.begin(), name);
		}
	}
}

ComponentFactory::ComponentFactory(int x, int y, ComponentBuilder& builder)
: ox(x), oy(y), grid(builder.getGridBox()), bounding(builder.getBoundBox()), producer(builder.build()) {}

Box2D ComponentFactory::getGridBox() const {
	return grid;
}

Box2D ComponentFactory::getBoundBox() const {
	return bounding;
}

GuiComponent* ComponentFactory::build(int x, int y) const {
	return producer(ox + x, oy + y);
}

/*
 * GuiComponent Builder is implemented in the header
 * file 'component.hpp' as it is a templated class
 */

