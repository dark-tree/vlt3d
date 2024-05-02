
#pragma once

#include "external.hpp"
#include "render/pipeline.hpp"
#include "buffer/allocator.hpp"

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

			// set clear color
			VkClearValue clearColor = {{{r, g, b, a}}};
			info.clearValueCount = 1;
			info.pClearValues = &clearColor;

			vkCmdBeginRenderPass(vk_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
			return *this;
		}

		CommandRecorder& bindPipeline(GraphicsPipeline& pipeline) {
			vkCmdBindPipeline(vk_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vk_pipeline);
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

		void done() {
			if (vkEndCommandBuffer(vk_buffer) != VK_SUCCESS) {
				throw std::runtime_error("vkEndCommandBuffer: Failed to record a command buffer!");
			}
		}

};
