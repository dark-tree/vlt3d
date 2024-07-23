
#include "pause.hpp"

PauseScreen::PauseScreen()
: GridScreen({10, 10, 32}) {}

void PauseScreen::buildModel(GuiComposed::Builder& builder) {
	builder.add(1, 0, GuiImage::of().box(3, 3).inset(0.05).sprite("assets/sprites/vkblob.png"));
	builder.then(Chain::AFTER, GuiText::of().text("Hello World!").box(6, 2).center().tint(50, 50, 50).italics());
	builder.then(Chain::BELOW, GuiButton::of().box(3, 1).inset(0.05).text("Okay").sprite("assets/sprites/button.png").then([&] (auto& stack) { logger::info("Okay pressed!"); exit(0); }));
	builder.then(Chain::AFTER, GuiButton::of().box(3, 1).inset(0.05).text("Cancel").sprite("assets/sprites/button.png").then([&] (auto& stack) { logger::info("Cancel pressed!"); remove(); }));
	builder.add(5, 5);
	builder.then(Chain::BELOW, GuiImage::of().box(3, 3).inset(0.05).sprite("assets/sprites/vkblob.png"));
	builder.add(7, 5, GuiLine::of().tint(50, 7, 7).to(3, 4).weight(2));
	builder.add(1, 7, GuiCheck::of().label("Click me!").inset(0.05).then([] (bool state) { logger::info("Checkbox is now: ", state); }));
}

InputResult PauseScreen::onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) {
	if (auto* key = event.as<KeyboardEvent>()) {
		if (key->isKeyReleased(GLFW_KEY_ESCAPE)) {
			remove();
			return InputResult::CONSUME;
		}
	}

	return GridScreen::onEvent(stack, input, event);
}

void PauseScreen::draw(ImmediateRenderer& renderer, InputContext& input, Camera& camera, bool focused) {
	if (focused) {
		input.setMouseCapture(false);
	}

	GridScreen::draw(renderer, input, camera, focused);
}

