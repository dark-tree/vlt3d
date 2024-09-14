
#pragma once

#include "external.hpp"
#include "render/pipeline.hpp"
#include "buffer/allocator.hpp"
#include "descriptor/descriptor.hpp"
#include "render/framebuffer.hpp"
#include "buffer/query.hpp"

class CommandRecorder {

	private:

		VkCommandBuffer vk_buffer;
		VkPipelineLayout vk_layout;

		RenderPassTracker tracer;

	public:

		CommandRecorder(VkCommandBuffer vk_buffer)
		: vk_buffer(vk_buffer), vk_layout(nullptr) {}

		CommandRecorder& beginRenderPass(RenderPass& render_pass, Framebuffer& framebuffer, VkExtent2D extent) {

			tracer.reset(render_pass);

			#if !defined(NDEBUG)
			std::string fullname = render_pass.debug_name + " Pass";
			pushDebugBlock(fullname.c_str(), render_pass.debug_color.toFloat());
			#endif

			VkRenderPassBeginInfo info {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.renderPass = render_pass.vk_pass;
			info.framebuffer = framebuffer.vk_buffer;

			info.renderArea.offset = {0, 0};
			info.renderArea.extent = extent;

			// attachments define their own clear values
			const std::vector<VkClearValue>& values = render_pass.values;
			info.clearValueCount = values.size();
			info.pClearValues = values.data();

			vkCmdBeginRenderPass(vk_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
			return *this;
		}

		CommandRecorder& resetQueryPool(QueryPool& pool) {
			vkCmdResetQueryPool(vk_buffer, pool.vk_pool, 0, pool.size());
			return *this;
		}

		CommandRecorder& queryTimestamp(QueryPool& pool, uint32_t index, VkPipelineStageFlagBits stage) {
			vkCmdWriteTimestamp(vk_buffer, stage, pool.vk_pool, index);
			return *this;
		}

		CommandRecorder& beginQuery(QueryPool& pool, uint32_t index) {
			vkCmdBeginQuery(vk_buffer, pool.vk_pool, index, 0);
			return *this;
		}

		CommandRecorder& endQuery(QueryPool& pool, uint32_t index) {
			vkCmdEndQuery(vk_buffer, pool.vk_pool, index);
			return *this;
		}

		CommandRecorder& beginConditional(Buffer& buffer, uint32_t offset) {
			VkConditionalRenderingBeginInfoEXT begin_info {};
			begin_info.sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
			begin_info.pNext = nullptr;
			begin_info.flags = 0;
			begin_info.buffer = buffer.vk_buffer;
			begin_info.offset = offset;

			vkCmdBeginConditionalRenderingEXT(vk_buffer, &begin_info);
			return *this;
		}

		CommandRecorder& endConditional() {
			vkCmdEndConditionalRenderingEXT(vk_buffer);
			return *this;
		}

		CommandRecorder& insertDebugLabel(const char* name) {
			VulkanDebug::insert(vk_buffer, name, {1.0f, 1.0f, 1.0f});
			return *this;
		}

		CommandRecorder& pushDebugBlock(const char* name, glm::vec3 color) {
			VulkanDebug::begin(vk_buffer, name, color);
			return *this;
		}

		CommandRecorder& popDebugBlock() {
			VulkanDebug::end(vk_buffer);
			return *this;
		}

		CommandRecorder& nextSubpass() {
			tracer.advance();
			vkCmdNextSubpass(vk_buffer, VK_SUBPASS_CONTENTS_INLINE);
			return *this;
		}

		CommandRecorder& bindPipeline(GraphicsPipeline& pipeline) {
			this->vk_layout = pipeline.vk_layout;
			vkCmdBindPipeline(vk_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vk_pipeline);
			return *this;
		}

		CommandRecorder& bindDescriptorSet(DescriptorSet& set) {
			vkCmdBindDescriptorSets(vk_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_layout, 0, 1, &set.vk_set, 0, nullptr);
			return *this;
		}

		CommandRecorder& writePushConstant(const PushConstant& constant, const void* data) {
			vkCmdPushConstants(vk_buffer, vk_layout, constant.flags, constant.offset, constant.size, data);
			return *this;
		}
		
		CommandRecorder& bindBuffer(const Buffer& buffer, VkDeviceSize offset = 0) {
			VkDeviceSize offsets[] = {offset};
			vkCmdBindVertexBuffers(vk_buffer, 0, 1, &buffer.vk_buffer, offsets);
			return *this;
		}

		CommandRecorder& draw(uint32_t vertices, uint32_t instances = 1, uint32_t vertexIndexOffset = 0, uint32_t instanceIndexOffset = 0) {
			vkCmdDraw(vk_buffer, vertices, instances, vertexIndexOffset, instanceIndexOffset);
			return *this;
		}

		CommandRecorder& endRenderPass() {
			tracer.end();
			vkCmdEndRenderPass(vk_buffer);

			#if !defined(NDEBUG)
			popDebugBlock();
			#endif
			return *this;
		}

		CommandRecorder& copyQueryToBuffer(Buffer& buffer, QueryPool& pool, int start, int end) {
			vkCmdCopyQueryPoolResults(vk_buffer, pool.vk_pool, start, end - start, buffer.vk_buffer, 0, sizeof(uint32_t), VK_QUERY_RESULT_WAIT_BIT);
			return *this;
		}

		CommandRecorder& copyBufferToBuffer(Buffer dst, Buffer src, size_t size) {
			VkBufferCopy region {};
			region.size = size;

			vkCmdCopyBuffer(vk_buffer, src.vk_buffer, dst.vk_buffer, 1, &region);
			return *this;
		}

		CommandRecorder& copyBufferToImage(Image dst, Buffer src, size_t offset, size_t width, size_t height, size_t layers, size_t level) {

			VkBufferImageCopy region {};
			region.bufferOffset = offset;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = level;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = layers;

			region.imageOffset = {0, 0, 0};
			region.imageExtent.width = width;
			region.imageExtent.height = height;
			region.imageExtent.depth = 1;

			vkCmdCopyBufferToImage(vk_buffer, src.vk_buffer, dst.vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			return *this;
		}

		CommandRecorder& transitionLayout(Image image, VkImageLayout dst, VkImageLayout src, size_t layers, size_t levels) {

			VkPipelineStageFlags src_stage = 0;
			VkPipelineStageFlags dst_stage = 0;

			VkImageMemoryBarrier barrier {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = src;
			barrier.newLayout = dst;

			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			barrier.image = image.vk_image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = levels;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = layers;

			if (src == VK_IMAGE_LAYOUT_UNDEFINED && dst == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}

			if (src == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && dst == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}

			// allow reading from already written to sections (the whole thing doesn't need to finish)
			VkDependencyFlags flags = VK_DEPENDENCY_BY_REGION_BIT;

			vkCmdPipelineBarrier(vk_buffer, src_stage, dst_stage, flags, 0, nullptr, 0, nullptr, 1, &barrier);
			return *this;
		}

		CommandRecorder& bufferTransferBarrier(VkPipelineStageFlags dst) {

			VkMemoryBarrier barrier {};
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

			VkPipelineStageFlags src = VK_PIPELINE_STAGE_TRANSFER_BIT;

			// allow reading from already written to sections (the whole thing doesn't need to finish)
			VkDependencyFlags flags = VK_DEPENDENCY_BY_REGION_BIT;

			vkCmdPipelineBarrier(vk_buffer, src, dst, flags, 1, &barrier, 0, nullptr, 0, nullptr);
			return *this;

		}

		void done() {
			if (vkEndCommandBuffer(vk_buffer) != VK_SUCCESS) {
				throw Exception {"Failed to record a command buffer!"};
			}
		}

};
