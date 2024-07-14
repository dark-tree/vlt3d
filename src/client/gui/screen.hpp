#pragma once

#include "external.hpp"
#include "window/input.hpp"
#include "client/renderer.hpp"

class Screen : public InputConsumer {

	private:

		bool removed;

		friend class ScreenStack;

	public:

		Screen();
		~Screen() override = default;

		void remove();
		virtual void draw(ImmediateRenderer& renderer, Camera& camera);

};
