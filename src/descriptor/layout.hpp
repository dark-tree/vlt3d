#pragma once

#include "external.hpp"

class Device;

struct DescriptorType {
	VkDescriptorType vk_type;
	bool enabled;

	inline DescriptorType()
	: enabled(false) {}
};

class DescriptorSetLayout {

	public:

		READONLY VkDevice vk_device;
		READONLY VkDescriptorSetLayout vk_layout;
		READONLY std::vector<DescriptorType> types;

	public:

		DescriptorSetLayout() = default;
		DescriptorSetLayout(VkDevice device, VkDescriptorSetLayout layout, const std::vector<DescriptorType>& types);

		/**
		 * Free the underlying vulkan object
		 */
		void close();

		/**
		 * Returns the VkDescriptorType of the specified binding index as defined in DescriptorSetLayoutBuilder
		 */
		VkDescriptorType getType(uint32_t index) const;

		/**
		 * Append all descriptor types used in this layout to the given list
		 */
		 void appendUsedTypes(std::vector<VkDescriptorType>& vector) const;
};

class DescriptorSetLayoutBuilder {

	private:

		READONLY VkDescriptorSetLayoutCreateFlags vk_flags;
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::vector<DescriptorType> types;
		ankerl::unordered_dense::segmented_set<uint32_t> indices;

		void addBindingTypeMapping(uint32_t index, VkDescriptorType type);

	public:

		DescriptorSetLayoutBuilder() = default;
		DescriptorSetLayoutBuilder(VkDescriptorSetLayoutCreateFlags flags = 0);

		/**
		 * Adds a new descriptor binding
		 *
		 * @param index the binding index, must match the one specified in the shader and the one in the allocated descriptor set
		 * @param type the type of the data that will be bound to this binding
		 * @param shader the shader stages where the binding will be made available
		 * @param count
		 */
		DescriptorSetLayoutBuilder& descriptor(uint32_t index, VkDescriptorType type, VkShaderStageFlags shader);

		/**
		 * Finish and build the layout object
		 */
		DescriptorSetLayout done(Device device) const;

		/**
		 * Get a new DescriptorSetLayoutBuilder
		 */
		static DescriptorSetLayoutBuilder begin(VkDescriptorSetLayoutCreateFlags flags = 0);

};