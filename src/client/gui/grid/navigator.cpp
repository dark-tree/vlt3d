
#include "navigator.hpp"

void GridNavigator::addNavigationNode(GuiComponent* node) {
	nodes.push_back(node);
}

void GridNavigator::next() {
	if (!nodes.empty()) index = (index + 1) % nodes.size();
}

void GridNavigator::stop() {
	index = -1;
}

bool GridNavigator::isFocused(GuiComponent* node) const {
	return index >= 0 && nodes[index] == node;
}