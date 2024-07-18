
#include "navigator.hpp"
#include "util/util.hpp"

void GridNavigator::addNavigationNode(GuiComponent* node) {
	nodes.push_back(node);
}

void GridNavigator::next() {
	if (!nodes.empty()) index = (index + 1) % nodes.size();
}

void GridNavigator::stop() {
	index = -1;
}

bool GridNavigator::isFocused(const GuiComponent* node) const {
	return index >= 0 && nodes[index] == node;
}

int GridNavigator::isNavigable(const GuiComponent* node) const {
	auto it = find(nodes.cbegin(), nodes.cend(), node);

	if (it == nodes.cend()) {
		return 0;
	}

	return std::distance(nodes.begin(), it) + 1;
}
