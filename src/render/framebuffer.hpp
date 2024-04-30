
#pragma once

#include "renderpass.hpp"
#include "view.hpp"
#include "setup/device.hpp"

class Framebuffer {

	public:

		READONLY VkFramebuffer vk_buffer;
		READONLY VkDevice vk_device;

	public:

		Framebuffer(VkFramebuffer vk_buffer, VkDevice vk_device)
		: vk_buffer(vk_buffer), vk_device(vk_device) {}

		void close() {
			vkDestroyFramebuffer(vk_device, vk_buffer, nullptr);
		}

		static Framebuffer build(Device& device, RenderPass& pass, const ImageView& view, uint32_t width, uint32_t height) {

			VkFramebufferCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.renderPass = pass.vk_pass;

			// TODO if we ever want more than once image per frame buffer this will fail up
			create_info.attachmentCount = 1;
			create_info.pAttachments = &view.vk_view;

			create_info.width = width;
			create_info.height = height;

			// TODO should this be passed by the image?
			create_info.layers = 1;

			VkFramebuffer framebuffer;

			if (vkCreateFramebuffer(device.vk_device, &create_info, nullptr, &framebuffer) != VK_SUCCESS) {
				throw std::runtime_error("vkCreateFramebuffer: Failed to create a framebuffer!");
			}

			return {framebuffer, device.vk_device};

		}

};
