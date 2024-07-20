#pragma once

#include "external.hpp"
#include "client/gui/screen.hpp"
#include "client/gui/grid/context.hpp"

#include "client/gui/component/button.hpp"
#include "client/gui/component/checkbox.hpp"
#include "client/gui/component/composed.hpp"
#include "client/gui/component/image.hpp"
#include "client/gui/component/line.hpp"
#include "client/gui/component/spacer.hpp"
#include "client/gui/component/text.hpp"

class GridScreen : public Screen {

	private:

		bool should_rebuild = true;
		GridContext context;

	protected:

		void rebuildModel();
		virtual void buildModel(GuiComposed::Builder& builder) = 0;

	public:

		GridScreen(GridContext context);
		~GridScreen() = default;

		InputResult onEvent(ScreenStack& stack, InputContext& input, const InputEvent& event) override;
		void draw(ImmediateRenderer &renderer, InputContext& input, Camera& camera, bool focused) override;

};
