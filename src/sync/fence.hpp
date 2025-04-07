#pragma once

#include "external.hpp"
#include "setup/callback.hpp"
#include "util/exception.hpp"

class Fence {

	public:

		READONLY VkDevice vk_device;
		READONLY VkFence vk_fence;

	public:

		Fence(VkDevice vk_device, bool signaled = false)
		: vk_device(vk_device) {

			VkFenceCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

			if (signaled) {
				create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			}

			if (vkCreateFence(vk_device, &create_info, AllocatorCallbackFactory::named("Fence"), &vk_fence) != VK_SUCCESS) {
				throw Exception {"Failed to create a fence!"};
			}
		}

		void close() {
			vkDestroyFence(vk_device, vk_fence, AllocatorCallbackFactory::named("Fence"));
		}

		void wait(size_t timeout = UINT64_MAX) {
			vkWaitForFences(vk_device, 1, &vk_fence, VK_TRUE, timeout);
		}

		void reset() {
			vkResetFences(vk_device, 1, &vk_fence);
		}

		void lock(size_t timeout = UINT64_MAX) {
			wait(timeout);
			reset();
		}

};
