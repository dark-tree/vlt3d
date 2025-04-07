
#include "composed.hpp"
#include "util/exception.hpp"
#include "client/gui/grid/context.hpp"
#include "client/gui/component/spacer.hpp"

/*
 * ComponentFactory
 */

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
 * GuiComposed
 */

GuiComposed::GuiComposed(Box2D box, const std::vector<std::shared_ptr<GuiComponent>>& components)
: GuiComponent(box), components(components) {}

void GuiComposed::draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) {
	for (auto& component : components) {
		component->draw(grid, input, renderer);
	}

	drawDebugOutline(grid, input, renderer, "GuiComposed", 0, 255, 255, 0.1);
}

bool GuiComposed::onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) {
	if (!GridContext::shouldAccept(grid.getScreenBox(bounding), input, event)) {
		return false;
	}

	bool any = false;

	for (auto& component : components) {
		any |= component->onEvent(grid, stack, input, event);
	}

	return any;
}

void GuiComposed::navigatorUpdate(GridNavigator& grid) {
	for (auto& component : components) {
		component->navigatorUpdate(grid);
	}
}

/*
 * GuiComposed Builder
 */

const ComponentFactory& GuiComposed::Builder::getLast() {
	if (children.empty()) {
		throw Exception {"Component chain needs to start with a unchained component!"};
	}

	return children.at(children.size() - 1);
}

void GuiComposed::Builder::append(ComponentFactory factory) {

	// expand our bounding box
	Box2D other = factory.getBoundBox().offset(x, y);
	combined = combined.empty() ? other : combined.envelop(other);

	// add the factory to the children list
	children.emplace_back(factory);
}

GuiComposed::Builder& GuiComposed::Builder::add(int x, int y) {
	GuiSpacer::Builder builder = GuiSpacer::of();
	return add(x, y, builder);
}

GuiComposed::Builder& GuiComposed::Builder::add(int x, int y, ComponentBuilder& builder) {
	this->x = x;
	this->y = y;
	append({x, y, builder});
	return *this;
}

GuiComposed::Builder& GuiComposed::Builder::then(Chain chain, ComponentBuilder& builder) {
	Box2D previous = getLast().getGridBox();
	Box2D current = builder.getGridBox();

	this->x = chain.nextX(x, previous.width(), current.width());
	this->y = chain.nextY(y, previous.height(), current.height());
	append({x, y, builder});
	return *this;
}

Box2D GuiComposed::Builder::getGridBox() const {
	return combined.round();
}

Box2D GuiComposed::Builder::getBoundBox() const {
	return combined;
}

ComponentProducer GuiComposed::Builder::build() const {
	return [box = this->getBoundBox(), children = this->children] (int x, int y) {
		std::vector<std::shared_ptr<GuiComponent>> cmps;

		for (const ComponentFactory& factory : children) {
			cmps.emplace_back(factory.build(x, y));
		}

		return new GuiComposed {box.offset(x, y), cmps};
	};
}