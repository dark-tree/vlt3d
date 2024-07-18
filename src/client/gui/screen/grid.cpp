
#include "grid.hpp"
#include "client/gui/component/composed.hpp"
#include "client/gui/component/image.hpp"
#include "client/gui/component/text.hpp"
#include "client/gui/component/button.hpp"

GridScreen::GridScreen() {

	ComponentProducer producer = GuiComposed::of()
		.add(1, 0, GuiImage::of().box(3, 3).inset(0.05).sprite("assets/sprites/vkblob.png"))
		.then(Chain::AFTER, GuiText::of().text("Hello World!").box(6, 2).center().tint(50, 50, 50).italics())
		.then(Chain::BELOW, GuiButton::of().box(3, 1).text("Okay").sprite("assets/sprites/button.png").then([&] (auto& stack) { logger::info("Okay pressed!"); remove(); }))
		.then(Chain::AFTER, GuiButton::of().box(3, 1).text("Cancel").sprite("assets/sprites/button.png").then([] (auto& stack) { logger::info("Cancel pressed!"); }))
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