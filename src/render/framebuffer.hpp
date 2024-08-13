
#pragma once

#include "pass.hpp"
#include "view.hpp"
#include "setup/device.hpp"

class Framebuffer {

	public:

		READONLY VkFramebuffer vk_buffer;
		READONLY VkDevice vk_device;
		READONLY uint32_t index;
		READONLY std::vector<VkImageView> owned;

	public:

		Framebuffer(VkFramebuffer vk_buffer, VkDevice vk_device, uint32_t index, std::vector<VkImageView> owned)
		: vk_buffer(vk_buffer), vk_device(vk_device), index(index), owned(owned) {}

		void close() {
			for (VkImageView view : owned) {
				vkDestroyImageView(vk_device, view, AllocatorCallbackFactory::named("View"));
			}

			vkDestroyFramebuffer(vk_device, vk_buffer, AllocatorCallbackFactory::named("Framebuffer"));
		}

};

class FramebufferBuilder {

	private:

		VkRenderPass pass;
		uint32_t width, height;
		std::vector<VkImageView> attachments;
		std::vector<VkImageView> owned;

	public:

		FramebufferBuilder(RenderPass& pass, VkExtent2D extent) {
			this->pass = pass.vk_pass;
			this->width = extent.width;
			this->height = extent.height;
		}

		void addAttachment(ImageView view, bool owning = false) {
			attachments.push_back(view.vk_view);

			if (owning) {
				owned.push_back(view.vk_view);
			}
		}

		Framebuffer build(Device& device, uint32_t index) {

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

			if (vkCreateFramebuffer(device.vk_device, &create_info, AllocatorCallbackFactory::named("Framebuffer"), &framebuffer) != VK_SUCCESS) {
				throw Exception {"Failed to create a framebuffer!"};
			}

			return {framebuffer, device.vk_device, index, owned};

		}

};
