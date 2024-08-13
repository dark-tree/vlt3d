#pragma once

#include "setup/device.hpp"
#include "kind.hpp"

/**
 * Class representing a single shader stage (like vertex shader or fragment shader)
 */
class ShaderModule {

	public:

		READONLY VkShaderModule vk_module;
		READONLY VkShaderStageFlagBits vk_stage;

	public:

		ShaderModule() = default;

		ShaderModule(Device& device, const uint32_t* data, uint32_t size, Kind kind)
		: vk_stage(kind.vulkan) {

			VkShaderModuleCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			create_info.codeSize = size;
			create_info.pCode = data;

			if (vkCreateShaderModule(device.vk_device, &create_info, AllocatorCallbackFactory::named("ShaderModule"), &vk_module) != VK_SUCCESS) {
				throw Exception {"Failed to create shader module!"};
			}

		}

		void close(Device& device) {
			vkDestroyShaderModule(device.vk_device, vk_module, AllocatorCallbackFactory::named("ShaderModule"));
		}

		VkPipelineShaderStageCreateInfo getStageConfig() const {

			VkPipelineShaderStageCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			create_info.stage = vk_stage;
			create_info.module = vk_module;
			create_info.pName = "main";

			return create_info;

		}

};
