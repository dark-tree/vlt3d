
#include "push.hpp"

/*
 * PushConstant
 */

PushConstant::PushConstant(VkPushConstantRange range)
: flags(range.stageFlags), offset(range.offset), size(range.size) {}

/*
 * PushConstantLayout
 */

PushConstantLayout::PushConstantLayout(const std::vector<VkPushConstantRange>& constants)
: constants(constants) {}

const VkPushConstantRange* PushConstantLayout::data() const {
	return constants.data();
}

uint32_t PushConstantLayout::size() const {
	return constants.size();
}

/*
 * PushConstantLayoutBuilder
 */

PushConstantLayoutBuilder& PushConstantLayoutBuilder::add(Kind kind, uint32_t bytes, PushConstant* constant) {
	return add(kind.vulkan, bytes, constant);
}

PushConstantLayoutBuilder& PushConstantLayoutBuilder::add(VkShaderStageFlags stage, uint32_t bytes, PushConstant* constant) {
	VkPushConstantRange range {};
	range.stageFlags = stage;
	range.size = bytes;
	range.offset = this->offset;

	this->offset += bytes;
	vk_constants.push_back(range);
	*constant = {range};
	return *this;
}

PushConstantLayout PushConstantLayoutBuilder::done() const {
	return {vk_constants};
}