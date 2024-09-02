
#include "layout.hpp"
#include "util/exception.hpp"
#include "setup/device.hpp"

/*
 * DescriptorSetLayout
 */

DescriptorSetLayout::DescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout, const std::vector<DescriptorType>& types)
: vk_device(device), vk_layout(layout), types(types) {}

void DescriptorSetLayout::close() {
	vkDestroyDescriptorSetLayout(vk_device, vk_layout, nullptr);
}

VkDescriptorType DescriptorSetLayout::getType(uint32_t index) const {
	if (index >= types.size()) {
		throw Exception {"Descriptor index out of defined range!"};
	}

	return types.at(index).vk_type;
}

void DescriptorSetLayout::appendUsedTypes(std::vector<VkDescriptorType>& vector) const {
	vector.reserve(types.size());

	for (DescriptorType type : types) {
		if (type.enabled) {
			vector.push_back(type.vk_type);
		}
	}
}

/*
 * DescriptorSetLayoutBuilder
 */

void DescriptorSetLayoutBuilder::addBindingTypeMapping(uint32_t index, VkDescriptorType type) {
	types.resize(index + 1);
	DescriptorType& descriptor = types[index];

	descriptor.vk_type = type;
	descriptor.enabled = true;
}

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(VkDescriptorSetLayoutCreateFlags flags)
: vk_flags(flags) {}

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::descriptor(uint32_t index, VkDescriptorType type, VkShaderStageFlags shader) {

	if (indices.contains(index)) {
		throw Exception {"Attempted to redefine descriptor set binding index!"};
	}

	VkDescriptorSetLayoutBinding binding {};
	binding.binding = index;
	binding.descriptorType = type;
	binding.stageFlags = shader;
	binding.descriptorCount = 1;

	bindings.push_back(binding);
	addBindingTypeMapping(index, type);
	indices.insert(index);

	return *this;
}

DescriptorSetLayout DescriptorSetLayoutBuilder::done(Device device) const {
	VkDescriptorSetLayout layout;

	VkDescriptorSetLayoutCreateInfo create_info {};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_info.flags = vk_flags;
	create_info.bindingCount = bindings.size();
	create_info.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device.vk_device, &create_info, nullptr, &layout) != VK_SUCCESS) {
		throw Exception {"Failed to create descriptor set!"};
	}

	return {device.vk_device, layout, types};
}

DescriptorSetLayoutBuilder DescriptorSetLayoutBuilder::begin(VkDescriptorSetLayoutCreateFlags flags) {
	return {flags};
}