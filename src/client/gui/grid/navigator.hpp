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
		void reset();

	public:

		/**
		 * Makes the node navigable, after a node is made navigable
		 * it can be focused by the user using keyboard inputs
		 */
		void addNavigationNode(GuiComponent* node);

		/**
		 * Check if a given component is focused
		 * Will return false if the given node isn't navigable
		 */
		bool isFocused(const GuiComponent* node) const;

		/**
		 * Check if a given component is navigable,
		 * Will return the nods navigation index (1 based) if it is navigable, 0 otherwise
		 */
		int isNavigable(const GuiComponent* node) const;

};
