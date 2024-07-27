#pragma once

class DescriptorSetLayout {

	public:

		READONLY VkDevice vk_device;
		READONLY VkDescriptorSetLayout vk_layout;

	public:

		DescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout)
		: vk_device(device), vk_layout(layout) {}

		void close() {
			vkDestroyDescriptorSetLayout(vk_device, vk_layout, nullptr);
		}

};

class DescriptorSetLayoutBuilder {

	private:

		READONLY VkDescriptorSetLayoutCreateFlags vk_flags;
		std::vector<VkDescriptorSetLayoutBinding> bindings;

	public:

		DescriptorSetLayoutBuilder(VkDescriptorSetLayoutCreateFlags flags = 0)
		: vk_flags(flags) {}

		DescriptorSetLayoutBuilder& descriptor(uint32_t index, VkDescriptorType type, VkShaderStageFlags shader, uint32_t count = 1) {
			VkDescriptorSetLayoutBinding binding {};
			binding.binding = index;
			binding.descriptorType = type;
			binding.stageFlags = shader;
			binding.descriptorCount = count;

			bindings.push_back(binding);
			return *this;
		}

		DescriptorSetLayout done(Device device) const {
			VkDescriptorSetLayout layout;

			VkDescriptorSetLayoutCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			create_info.flags = vk_flags;
			create_info.bindingCount = bindings.size();
			create_info.pBindings = bindings.data();

			if (vkCreateDescriptorSetLayout(device.vk_device, &create_info, nullptr, &layout) != VK_SUCCESS) {
				throw std::runtime_error {"vkCreateDescriptorSetLayout: Failed to create descriptor set!"};
			}

			return {device.vk_device, layout};
		}

		inline static DescriptorSetLayoutBuilder begin() {
			return {};
		}

};