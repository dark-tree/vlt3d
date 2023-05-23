#pragma once

#include "external.hpp"

class Semaphore {

	public:

		READONLY VkDevice vk_device;
		READONLY VkSemaphore vk_semaphore;

	public:

		Semaphore(VkDevice vk_device)
		: vk_device(vk_device) {

			VkSemaphoreCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(vk_device, &create_info, nullptr, &vk_semaphore) != VK_SUCCESS) {
				throw std::runtime_error("vkCreateSemaphore: Failed to create a semaphore!");
			}
		}

		void close() {
			vkDestroySemaphore(vk_device, vk_semaphore, nullptr);
		}

};
