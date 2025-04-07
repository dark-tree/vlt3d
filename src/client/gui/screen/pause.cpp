
#include "pause.hpp"

PauseScreen::PauseScreen()
: GridScreen({12, 10, 32}) {
	AllocatorCallbackFactory::print();
}

void PauseScreen::buildModel(GuiComposed::Builder& builder) {
	builder.add(0, 0, GuiText::of().text("Main Menu").inset(0.2, 0, 0, 0).box(12, 1).center().tint(80, 80, 80));
	builder.then(Chain::BELOW, GuiLine::of().weight(1).to(12, 0).tint(50, 50, 50));
	builder.then(Chain::BELOW, GuiSpacer::of().box(12, 1));

	auto quit = GuiComposed::of()
		.add(0, 0, GuiButton::of().box(3, 1).inset(0.05).text("Quit").sprite("button").then([&] (auto& stack) {
			logger::info("Quit clicked!");
			std::quick_exit(0);
		}))
		.then(Chain::AFTER, GuiText::of().text("Quit to desktop").inset(0, 0, 0, 0.5).align(VerticalAlignment::CENTER).tint(50, 50, 50));

	auto cancel = GuiComposed::of()
		.add(0, 0, GuiButton::of().box(3, 1).inset(0.05).text("Cancel").sprite("button").then([&] (auto& stack) {
			logger::info("Cancel clicked!");
			remove();
		}))
		.then(Chain::AFTER, GuiText::of().text("Return to the game").inset(0, 0, 0, 0.5).align(VerticalAlignment::CENTER).tint(50, 50, 50));

	builder.then(Chain::BELOW, quit);
	builder.then(Chain::BELOW, cancel);
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

