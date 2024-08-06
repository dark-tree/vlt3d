
#include "component.hpp"
#include "client/gui/grid/context.hpp"

GuiComponent::GuiComponent(Box2D box)
: bounding(box) {}

void GuiComponent::drawDebugOutline(GridContext& grid, InputContext& input, ImmediateRenderer& renderer, const char* name, uint8_t r, uint8_t g, uint8_t b, float outset) const {
	if (grid.isDebugMode(input)) {
		Box2D screen = grid.getScreenBox(this->bounding.inset(-outset));

		renderer.setAlignment(HorizontalAlignment::LEFT);
		renderer.setFontTilt(0);

		renderer.setTint(r, g, b);
		renderer.setLineSize(1);
		renderer.drawLine(screen.x1, screen.y1, screen.x2, screen.y1);
		renderer.drawLine(screen.x1, screen.y1, screen.x1, screen.y2);
		renderer.drawLine(screen.x2, screen.y1, screen.x2, screen.y2);
		renderer.drawLine(screen.x1, screen.y2, screen.x2, screen.y2);

		if (input.isMouseWithin(screen)) {
			renderer.setAlignment(VerticalAlignment::BOTTOM);
			renderer.setFontSize(2);
			renderer.drawText(screen.begin(), name);
		}

		if (int i = grid.isNavigable(this)) {
			renderer.setAlignment(VerticalAlignment::TOP);
			renderer.setTint(255, 255, 255);
			renderer.setFontSize(1);

			// this emits a bogus warning caused by a compiler bug on GCC 12
			// see: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105329
			renderer.drawText(screen.x1 + 3, screen.y1 + 3, "#" + std::to_string(i));
		}
	}
}

/*
 * GuiComponent Builder is implemented in the header
 * file 'component.hpp' as it is a templated class
 */

