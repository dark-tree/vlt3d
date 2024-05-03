#pragma once

class DescriptorSetBuilder {

	private:

		READONLY VkDevice vk_device;
		READONLY VkDescriptorSetLayoutCreateFlags vk_flags;

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::vector<VkDescriptorSetLayout>& sets;

	public:

		DescriptorSetBuilder(VkDevice device, VkDescriptorSetLayoutCreateFlags flags, std::vector<VkDescriptorSetLayout>& sets)
		: vk_device(device), vk_flags(flags), sets(sets) {}

		DescriptorSetBuilder& descriptor(uint32_t index, VkDescriptorType type, VkShaderStageFlags shader, uint32_t count = 1) {
			VkDescriptorSetLayoutBinding binding {};
			binding.binding = index;
			binding.descriptorType = type;
			binding.stageFlags = shader;
			binding.descriptorCount = count;

			bindings.push_back(binding);
			return *this;
		}

		VkDescriptorSetLayout done() const {
			VkDescriptorSetLayout layout;

			VkDescriptorSetLayoutCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			create_info.flags = vk_flags;
			create_info.bindingCount = bindings.size();
			create_info.pBindings = bindings.data();

			if (vkCreateDescriptorSetLayout(vk_device, &create_info, nullptr, &layout) != VK_SUCCESS) {
				throw std::runtime_error {"vkCreateDescriptorSetLayout: Failed to create descriptor set!"};
			}

			sets.push_back(layout);
			return layout;
		}

};