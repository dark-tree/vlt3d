
#include "grid.hpp"

#include <utility>
#include "client/gui/component/composed.hpp"
#include "client/gui/component/image.hpp"
#include "client/gui/component/text.hpp"
#include "client/gui/component/button.hpp"
#include "client/gui/component/line.hpp"
#include "client/gui/component/checkbox.hpp"

void GridScreen::rebuildModel() {
	this->should_rebuild = true;
}

GridScreen::GridScreen(GridContext context)
: context(std::move(context)) {}

InputResult GridScreen::onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) {
	return context.onEvent(stack, input, event);
}

void GridScreen::draw(ImmediateRenderer& renderer, InputContext& input, Camera& camera, bool focused) {
	if (should_rebuild) {
		should_rebuild = false;
		GuiComposed::Builder builder = GuiComposed::of();
		buildModel(builder);
		context.setModel(builder.build());
		logger::info("Rebuild model for screen ", this);
	}

	context.draw(renderer, input);
}