
#include "component.hpp"
#include "client/gui/grid/context.hpp"

GuiComponent::GuiComponent(Box2D box)
: bounding(box) {}

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

