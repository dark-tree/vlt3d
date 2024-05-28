
#pragma once

#include "pass.hpp"
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

};

class FramebufferBuilder {

	private:

		VkRenderPass pass;
		uint32_t width, height;
		std::vector<VkImageView> attachments;

	public:

		FramebufferBuilder(RenderPass& pass, uint32_t width, uint32_t height) {
			this->pass = pass.vk_pass;
			this->width = width;
			this->height = height;
		}

		void addAttachment(ImageView view) {
			attachments.push_back(view.vk_view);
		}

		Framebuffer build(Device& device) {

			VkFramebufferCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.renderPass = pass;

			create_info.attachmentCount = attachments.size();
			create_info.pAttachments = attachments.data();
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
