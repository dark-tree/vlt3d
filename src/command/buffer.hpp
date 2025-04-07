
#pragma once

#include "external.hpp"
#include "recorder.hpp"
#include "submiter.hpp"

class CommandBuffer {

	public:

		enum Level {
			PRIMARY   = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			SECONDARY = VK_COMMAND_BUFFER_LEVEL_SECONDARY
		};

		READONLY VkCommandPool vk_pool;
		READONLY VkCommandBuffer vk_buffer;
		READONLY VkDevice vk_device;

	public:

		CommandBuffer(VkCommandPool vk_pool, VkCommandBuffer vk_buffer, VkDevice vk_device)
		: vk_pool(vk_pool), vk_buffer(vk_buffer), vk_device(vk_device) {}

		CommandRecorder record(VkCommandBufferUsageFlags flags = 0, const VkCommandBufferInheritanceInfo* parent = nullptr) {

			vkResetCommandBuffer(vk_buffer, 0);

			VkCommandBufferBeginInfo info {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = flags;
			info.pInheritanceInfo = parent;

			if (vkBeginCommandBuffer(vk_buffer, &info) != VK_SUCCESS) {
				throw Exception {"Failed to begin recording a command buffer!"};
			}

			return {vk_buffer};

		}

		CommandSubmiter submit() {
			return {vk_buffer};
		}

		void close() {
			vkFreeCommandBuffers(vk_device, vk_pool, 1, &vk_buffer);
		}

};
