
#pragma once

#include "external.hpp"
#include "render/pipeline.hpp"
#include "buffer/allocator.hpp"
#include "descriptor/descriptor.hpp"

class CommandRecorder {

	private:

		VkCommandBuffer vk_buffer;

	public:

		CommandRecorder(VkCommandBuffer vk_buffer)
		: vk_buffer(vk_buffer) {}

		// TODO clean this one
		CommandRecorder& beginRenderPass(RenderPass& render_pass, Framebuffer& framebuffer, VkExtent2D extent, float r, float g, float b, float a) {

			VkRenderPassBeginInfo info {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = render_pass.vk_pass;
			info.framebuffer = framebuffer.vk_buffer;

			info.renderArea.offset = {0, 0};
			info.renderArea.extent = extent;

			// TODO set clear color
			VkClearValue clearColor[2] = {0};
			clearColor[0].color = {r, g, b, a};
			clearColor[1].depthStencil = {1.0f, 0};

			info.clearValueCount = 2;
			info.pClearValues = clearColor;

			vkCmdBeginRenderPass(vk_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
			return *this;
		}

		CommandRecorder& bindPipeline(GraphicsPipeline& pipeline) {
			vkCmdBindPipeline(vk_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vk_pipeline);
			return *this;
		}

		CommandRecorder& bindDescriptorSet(GraphicsPipeline& pipeline, DescriptorSet& set) {
			vkCmdBindDescriptorSets(vk_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vk_layout, 0, 1, &set.vk_set, 0, nullptr);
			return *this;
		}

		CommandRecorder& setDynamicViewport(int x, int y, uint32_t width, uint32_t height, float min_depth = 0.0f, float max_depth = 1.0f) {

			VkViewport viewport {};
			viewport.x = (float) x;
			viewport.y = (float) y;
			viewport.width = (float) width;
			viewport.height = (float) height;
			viewport.minDepth = min_depth;
			viewport.maxDepth = max_depth;

			vkCmdSetViewport(vk_buffer, 0, 1, &viewport);
			return *this;
		}

		CommandRecorder& setDynamicScissors(int x, int y, uint32_t width, uint32_t height) {

			VkRect2D scissor {};
			scissor.offset = {x, y};
			scissor.extent = {width, height};

			vkCmdSetScissor(vk_buffer, 0, 1, &scissor);
			return *this;
		}
		
		CommandRecorder& bindBuffer(Buffer& buffer) {
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(vk_buffer, 0, 1, &buffer.vk_buffer, offsets);
			return *this;
		}

		CommandRecorder& draw(uint32_t vertices, uint32_t instances = 1, uint32_t vertexIndexOffset = 0, uint32_t instanceIndexOffset = 0) {
			vkCmdDraw(vk_buffer, vertices, instances, vertexIndexOffset, instanceIndexOffset);
			return *this;
		}

		CommandRecorder& endRenderPass() {
			vkCmdEndRenderPass(vk_buffer);
			return *this;
		}

		CommandRecorder& copyBufferToBuffer(Buffer dst, Buffer src, size_t size) {
			VkBufferCopy region {};
			region.size = size;

			vkCmdCopyBuffer(vk_buffer, src.vk_buffer, dst.vk_buffer, 1, &region);
			return *this;
		}

		CommandRecorder& copyBufferToImage(Image dst, Buffer src, size_t width, size_t height, size_t depth = 1) {

			VkBufferImageCopy region {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = {0, 0, 0};
			region.imageExtent.width = width;
			region.imageExtent.height = height;
			region.imageExtent.depth = depth;

			vkCmdCopyBufferToImage(vk_buffer, src.vk_buffer, dst.vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			return *this;
		}

		CommandRecorder& transitionLayout(Image image, VkImageLayout dst, VkImageLayout src) {

			VkImageMemoryBarrier barrier {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = src;
			barrier.newLayout = dst;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			barrier.image = image.vk_image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			barrier.srcAccessMask = 0; // TODO
			barrier.dstAccessMask = 0; // TODO

			vkCmdPipelineBarrier(vk_buffer, 0 /* TODO */, 0 /* TODO */, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			return *this;
		}

		void done() {
			if (vkEndCommandBuffer(vk_buffer) != VK_SUCCESS) {
				throw std::runtime_error("vkEndCommandBuffer: Failed to record a command buffer!");
			}
		}

};
