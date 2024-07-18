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
		bool isFocused(const GuiComponent* node) const;
		int isNavigable(const GuiComponent* pComponent) const;

};
