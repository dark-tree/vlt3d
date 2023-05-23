
#pragma once

#include "external.hpp"
#include "setup/queues.hpp"
#include "buffer.hpp"

class CommandPool {

	public:

		READONLY VkCommandPool vk_pool;
		READONLY VkDevice vk_device;

	public:

		CommandPool(VkCommandPool vk_pool, VkDevice vk_device)
		: vk_pool(vk_pool), vk_device(vk_device) {}

		void close() {
			vkDestroyCommandPool(vk_device, vk_pool, nullptr);
		}

		CommandBuffer allocate(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) {

			VkCommandBufferAllocateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			create_info.commandPool = vk_pool;
			create_info.level = level;
			create_info.commandBufferCount = 1;

			VkCommandBuffer buffer;

			if (vkAllocateCommandBuffers(vk_device, &create_info, &buffer) != VK_SUCCESS) {
				throw std::runtime_error("vkAllocateCommandBuffers: Failed to allocate a command buffer!");
			}

			return {vk_pool, buffer, vk_device};

		}

		static CommandPool build(Device& device, QueueInfo info, bool transient) {

			VkCommandPoolCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			create_info.queueFamilyIndex = info.index;

			if (transient) {
				create_info.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
			}

			VkCommandPool pool;

			if (vkCreateCommandPool(device.vk_device, &create_info, nullptr, &pool) != VK_SUCCESS) {
				throw std::runtime_error("vkCreateCommandPool: Failed to create command pool!");
			}

			return {pool, device.vk_device};

		}

};
