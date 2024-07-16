#pragma once

#include "external.hpp"
#include "util/box.hpp"
#include "client/gui/grid/inset.hpp"
#include "client/gui/grid/chain.hpp"

class GridContext;
class ScreenStack;
class InputContext;
class InputEvent;
class ImmediateRenderer;

using ComponentProducer = std::function<class GuiComponent*(int, int)>;

struct ComponentBuilder {

	virtual ~ComponentBuilder() = default;

	virtual Box2D getGridBox() const = 0;
	virtual Box2D getBoundBox() const = 0;
	virtual ComponentProducer build() const = 0;

};

template <class T>
struct BoundComponentBuilder : public ComponentBuilder {

	protected:

		int width = 1;
		int height = 1;
		Inset insets;

	public:

		T& box(int w, int h) {
			this->width = w;
			this->height = h;
			return dynamic_cast<T&>(*this);
		}

		T& inset(float margin) {
			return inset(margin, margin);
		}

		T& inset(float vertical, float horizontal) {
			return inset(vertical, vertical, horizontal, horizontal);
		}

		T& inset(float top, float bottom, float right, float left) {
			insets.top = top;
			insets.bottom = bottom;
			insets.right = right;
			insets.left = left;
			return dynamic_cast<T&>(*this);
		}

		Box2D getGridBox() const final {
			return {0, 0, (float) width, (float) height};
		}

		Box2D getBoundBox() const final {
			return {insets.left, insets.bottom, width - insets.right, height - insets.top};
		}

};

struct ComponentFactory {

	private:

		int ox, oy;
		Box2D grid, bounding;
		ComponentProducer producer;

	public:

		ComponentFactory(int x, int y, ComponentBuilder& builder);

		Box2D getGridBox() const;
		Box2D getBoundBox() const;
		GuiComponent* build(int x, int y) const;

};

class GuiComponent {

	protected:

		READONLY Box2D bounding;

	protected:

		GuiComponent(Box2D box);

	public:

		virtual ~GuiComponent() = default;

		void drawDebugOutline(GridContext& grid, InputContext& input, ImmediateRenderer& renderer, const char* name, uint8_t r, uint8_t g, uint8_t b, float outset = 0) const;
		virtual void draw(GridContext& grid, InputContext& input, ImmediateRenderer& renderer) = 0;
		virtual bool onEvent(GridContext& grid, ScreenStack& stack, InputContext& input, const InputEvent& event) = 0;

};