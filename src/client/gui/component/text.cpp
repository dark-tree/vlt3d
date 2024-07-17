
#include "text.hpp"
#include "client/gui/grid/context.hpp"

GuiText::GuiText(Box2D box, Color color, float size, const std::string& text, VerticalAlignment vertical, HorizontalAlignment horizontal)
: GuiComponent(box), color(color), size(size), text(text), vertical(vertical), horizontal(horizontal) {}

void GuiText::draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) {
	Box2D box = grid.getScreenBox(bounding);
	glm::vec2 extend = {box.width(), box.height()};

	renderer.setAlignment(vertical);
	renderer.setAlignment(horizontal);
	renderer.setTint(color);
	renderer.setFontSize(size);
	renderer.setFontTilt(tilt);
	renderer.drawText(box.begin(), text, extend);

	drawDebugOutline(grid, input, renderer, "GuiText", 220, 220, 220);
}

bool GuiText::onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) {
	return false;
}

GuiText::Builder& GuiText::Builder::tint(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	this->color = Color {r, g, b, a};
	return *this;
}

GuiText::Builder& GuiText::Builder::text(const std::string& text) {
	this->string = text;
	return *this;
}

GuiText::Builder& GuiText::Builder::size(float font_size) {
	this->font_size = font_size;
	return *this;
}

GuiText::Builder& GuiText::Builder::italics(float font_tilt) {
	this->tilt = font_tilt;
	return *this;
}

GuiText::Builder& GuiText::Builder::align(VerticalAlignment alignment) {
	this->vertical = alignment;
	return *this;
}

GuiText::Builder& GuiText::Builder::align(HorizontalAlignment alignment) {
	this->horizontal = alignment;
	return *this;
}

GuiText::Builder& GuiText::Builder::align(VerticalAlignment vertical, HorizontalAlignment alignment) {
	return align(vertical).align(alignment);
}

GuiText::Builder& GuiText::Builder::center() {
	return align(VerticalAlignment::CENTER, HorizontalAlignment::CENTER);
}

GuiText::Builder& GuiText::Builder::left() {
	return align(VerticalAlignment::CENTER, HorizontalAlignment::LEFT);
}

ComponentProducer GuiText::Builder::build() const {
	return [box = this->getBoundBox(), color = this->color, size = this->font_size, text = this->string, va = this->vertical, ha = this->horizontal] (int x, int y) {
		return new GuiText {box.offset(x, y), color, size, text, va, ha};
	};
}