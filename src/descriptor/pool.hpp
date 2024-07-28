
#pragma once

#include "external.hpp"
#include "descriptor.hpp"

class DescriptorPool {

	public:

		READONLY VkDescriptorPool vk_pool;
		READONLY VkDevice vk_device;

	public:

		DescriptorPool() = default;
		DescriptorPool(VkDescriptorPool vk_pool, VkDevice vk_device)
		: vk_pool(vk_pool), vk_device(vk_device) {}

		void close() {
			vkDestroyDescriptorPool(vk_device, vk_pool, nullptr);
		}

		DescriptorSet allocate(DescriptorSetLayout layout) const {
			VkDescriptorSet set;

			VkDescriptorSetAllocateInfo alloc_info {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.pSetLayouts = &layout.vk_layout;
			alloc_info.descriptorSetCount = 1;
			alloc_info.descriptorPool = vk_pool;

			if (vkAllocateDescriptorSets(vk_device, &alloc_info, &set) != VK_SUCCESS) {
				throw Exception {"Failed to allocate descriptor set!"};
			}

			return {vk_device, set};
		}

		void reset() {
			vkResetDescriptorPool(vk_device, vk_pool, 0);
		}

};

class DescriptorPoolBuilder {

	private:

		std::vector<VkDescriptorPoolSize> sizes;

	public:

		DescriptorPoolBuilder& add(int count, VkDescriptorType type) {
			VkDescriptorPoolSize size {};
			size.descriptorCount = count;
			size.type = type;

			sizes.push_back(size);
			return *this;
		}

		DescriptorPool done(Device device, uint32_t sets) const {
			VkDescriptorPoolCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			create_info.poolSizeCount = sizes.size();
			create_info.pPoolSizes = sizes.data();
			create_info.maxSets = sets;

			VkDescriptorPool pool;

			if (vkCreateDescriptorPool(device.vk_device, &create_info, nullptr, &pool) != VK_SUCCESS) {
				throw Exception {"Failed to create descriptor pool!"};
			}

			return {pool, device.vk_device};
		}

		inline static DescriptorPoolBuilder begin() {
			return {};
		}

};