
#include "grid.hpp"
#include "client/gui/component/composed.hpp"
#include "client/gui/component/image.hpp"

GridScreen::GridScreen() {

	ComponentProducer producer = GuiComposed::of()
		.add(1, 0, GuiImage::of().box(3, 3).inset(0.05).sprite("assets/sprites/vkblob.png"))
		.add(5, 5)
		.then(Chain::BELOW, GuiImage::of().box(3, 3).inset(0.05).sprite("assets/sprites/vkblob.png"))
		.build();

	context.root.reset(producer(0, 0));
}

InputResult GridScreen::onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) {
	return context.onEvent(stack, input, event);
}

void GridScreen::draw(ImmediateRenderer& renderer, InputContext& input, Camera& camera) {
	context.draw(renderer, input);
}