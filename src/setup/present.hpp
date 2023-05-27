#pragma once

#include "external.hpp"

class PresentResult {

	public:

		const VkResult vk_result;

	public:

		PresentResult(VkResult vk_result)
		: vk_result(vk_result) {}

		bool shouldReplace() const {
			return mustReplace() || vk_result == VK_SUBOPTIMAL_KHR;
		}

		bool mustReplace() const {
			return vk_result == VK_ERROR_OUT_OF_DATE_KHR;
		}

};
