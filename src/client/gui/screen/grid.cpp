
#include "grid.hpp"
#include "client/gui/component/composed.hpp"
#include "client/gui/component/image.hpp"
#include "client/gui/component/text.hpp"
#include "client/gui/component/button.hpp"
#include "client/gui/component/line.hpp"

GridScreen::GridScreen() {

	ComponentProducer producer = GuiComposed::of()
		.add(1, 0, GuiImage::of().box(3, 3).inset(0.05).sprite("assets/sprites/vkblob.png"))
		.then(Chain::AFTER, GuiText::of().text("Hello World!").box(6, 2).center().tint(50, 50, 50).italics())
		.then(Chain::BELOW, GuiButton::of().box(3, 1).text("Okay").sprite("assets/sprites/button.png").then([&] (auto& stack) { logger::info("Okay pressed!"); remove(); }))
		.then(Chain::AFTER, GuiButton::of().box(3, 1).text("Cancel").sprite("assets/sprites/button.png").then([] (auto& stack) { logger::info("Cancel pressed!"); }))
		.add(5, 5)
		.then(Chain::BELOW, GuiImage::of().box(3, 3).inset(0.05).sprite("assets/sprites/vkblob.png"))
		.add(7, 5, GuiLine::of().tint(50, 7, 7).to(3, 4).weight(2))
		.build();

	context.root.reset(producer(0, 0));
	context.root->navigatorUpdate(context);
}

InputResult GridScreen::onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) {
	return context.onEvent(stack, input, event);
}

void GridScreen::draw(ImmediateRenderer& renderer, InputContext& input, Camera& camera) {
	context.draw(renderer, input);
}