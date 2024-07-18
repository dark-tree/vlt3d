#pragma once

#include "external.hpp"
#include "client/gui/component/component.hpp"

class GridNavigator {

	private:

		int index = -1;
		std::vector<GuiComponent*> nodes;

	protected:

		void next();
		void stop();

	public:

		void addNavigationNode(GuiComponent* node);
		bool isFocused(GuiComponent* node) const;

};
