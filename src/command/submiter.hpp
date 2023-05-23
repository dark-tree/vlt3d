
#pragma once

#include "external.hpp"
#include "render/pipeline.hpp"
#include "sync/semaphore.hpp"
#include "sync/fence.hpp"

class CommandSubmiter {

	private:

		VkCommandBuffer vk_buffer;
		VkFence finished_fence = VK_NULL_HANDLE;

		std::vector<VkSemaphore> wait_semaphores;
		std::vector<VkPipelineStageFlags> stages;
		std::vector<VkSemaphore> reset_semaphores;

	public:

		CommandSubmiter(VkCommandBuffer vk_buffer)
		: vk_buffer(vk_buffer) {}

		CommandSubmiter& awaits(Semaphore& semaphore, VkPipelineStageFlags stage) {
			wait_semaphores.push_back(semaphore.vk_semaphore);
			stages.push_back(stage);
			return *this;
		}

		CommandSubmiter& unlocks(Semaphore& semaphore) {
			reset_semaphores.push_back(semaphore.vk_semaphore);
			return *this;
		}

		CommandSubmiter& onFinished(Fence& fence) {
			finished_fence = fence.vk_fence;
			return *this;
		}

		void done(VkQueue queue) {

			VkSubmitInfo info {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			info.commandBufferCount = 1;
			info.pCommandBuffers = &vk_buffer;

			// wait for semaphores
			info.waitSemaphoreCount = (uint32_t) wait_semaphores.size();
			info.pWaitSemaphores = wait_semaphores.data();
			info.pWaitDstStageMask = stages.data();

			// unlock semaphore
			info.signalSemaphoreCount = (uint32_t) reset_semaphores.size();
			info.pSignalSemaphores = reset_semaphores.data();

			if (vkQueueSubmit(queue, 1, &info, finished_fence) != VK_SUCCESS) {
				throw std::runtime_error("failed to submit draw command buffer!");
			}

		}

};
