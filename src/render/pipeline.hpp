
#pragma once

#include "external.hpp"
#include "util/format.hpp"
#include "renderpass.hpp"
#include "setup/features.hpp"

#define ASSERT_FEATURE(test, device, feature) if ((test) && !device.features.has##feature ()) { throw std::runtime_error("feature '" #feature "' not enabled on this device!"); }

enum GlphBlendMode {
	GLPH_BLEND_BITWISE = 1,
	GLPH_BLEND_ENABLED = 2,
	GLPH_BLEND_DISABLED = 3
};

class GraphicsPipeline {

	public:

		READONLY VkPipeline vk_pipeline;
		READONLY VkPipelineLayout vk_layout;
		READONLY VkDevice vk_device;

	public:

		GraphicsPipeline(VkPipeline vk_pipeline, VkPipelineLayout vk_layout, VkDevice vk_device)
		: vk_pipeline(vk_pipeline), vk_layout(vk_layout), vk_device(vk_device) {}

		void close() {
			vkDestroyPipeline(vk_device, vk_pipeline, nullptr);
			vkDestroyPipelineLayout(vk_device, vk_layout, nullptr);
		}

};

class GraphicsPipelineBuilder {

	private:

		std::vector<VkDynamicState> dynamics;
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;
		std::vector<VkPipelineShaderStageCreateInfo> stages;

		VkPipelineViewportStateCreateInfo view {};
		VkPipelineDynamicStateCreateInfo dynamic {};
		VkPipelineVertexInputStateCreateInfo input {};
		VkPipelineLayoutCreateInfo layout {};
		VkPipelineInputAssemblyStateCreateInfo assembly {};
		VkPipelineRasterizationStateCreateInfo rasterizer {};
		VkPipelineMultisampleStateCreateInfo multisampling {};
		VkPipelineColorBlendAttachmentState attachment {};
		VkPipelineColorBlendStateCreateInfo blending {};

		VkViewport viewport {};
		VkRect2D scissor {};
		VkRenderPass pass;
		int subpass = -1;
		Device& device;

		void finalize() {

			input.vertexBindingDescriptionCount = (uint32_t) bindings.size();
			input.pVertexBindingDescriptions = bindings.data();

			input.vertexAttributeDescriptionCount = (uint32_t) attributes.size();
			input.pVertexAttributeDescriptions = attributes.data();

			dynamic.dynamicStateCount = (uint32_t) dynamics.size();
			dynamic.pDynamicStates = dynamics.data();

		}

	public:

		GraphicsPipelineBuilder(Device& device)
		: device(device) {

			view.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			view.viewportCount = 1;
			view.pViewports = &viewport;
			view.scissorCount = 1;
			view.pScissors = &scissor;

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineColorBlendStateCreateInfo.html
			dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineVertexInputStateCreateInfo.html
			input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineLayoutCreateInfo.html
			layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			layout.setLayoutCount = 0;
			layout.pSetLayouts = nullptr;
			layout.pushConstantRangeCount = 0;
			layout.pPushConstantRanges = nullptr;
			// TODO

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineInputAssemblyStateCreateInfo.html
			assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			setPrimitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineRasterizationStateCreateInfo.html
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			setDepthClamp(false);
			setDepthBias(false);
			setPolygonMode(VK_POLYGON_MODE_FILL);
			setLineWidth(1.0f);
			setCulling(false);

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineMultisampleStateCreateInfo.html
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			// TODO

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineColorBlendStateCreateInfo.html
			blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			blending.attachmentCount = 1;
			blending.pAttachments = &attachment;
			setBlendConstants(0.0f, 0.0f, 0.0f, 0.0f);
			setWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
			setBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
			setBlendAlphaFunc(VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ZERO);
			setBlendBitwiseFunc(VK_LOGIC_OP_COPY);
			setBlendMode(GLPH_BLEND_DISABLED);

		}

	// dynamic configuration

		template<typename... DynamicState>
		void setDynamics(DynamicState... states) {
			dynamics = { states... };
		}

	// view configuration

		void setViewport(int x, int y, uint32_t width, uint32_t height, float min_depth = 0.0f, float max_depth = 1.0f) {
			viewport.x = (float) x;
			viewport.y = (float) y;
			viewport.width = (float) width;
			viewport.height = (float) height;
			viewport.minDepth = min_depth;
			viewport.maxDepth = max_depth;
		}

		void setScissors(int x, int y, uint32_t width, uint32_t height) {
			scissor.offset = {x, y};
			scissor.extent = {width, height};
		}

	// vertex configuration

		void addAttribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset) {

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVertexInputAttributeDescription.html
			VkVertexInputAttributeDescription description {};
			description.location = location;
			description.binding = binding;
			description.format = format;
			description.offset = offset;

			attributes.push_back(description);
		}

		void addBinding(uint32_t binding, uint32_t stride, VkVertexInputRate rate) {

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVertexInputBindingDescription.html
			VkVertexInputBindingDescription description {};
			description.binding = binding;
			description.stride = stride;
			description.inputRate = rate;

			bindings.push_back(description);
		}

	// layout configuration

		// https://registry.khronos.org/vulkan/site/guide/latest/push_constants.html
		// TODO

	// assembly configuration

		void setPrimitive(VkPrimitiveTopology topology, bool enable_restart = false) {
			assembly.topology = topology;
			assembly.primitiveRestartEnable = enable_restart;
		}

	// rasterizer configuration

		void setDepthClamp(bool enable) {
			ASSERT_FEATURE(enable, device, DepthClamp);
			rasterizer.depthClampEnable = enable;
		}

		void setDepthBias(bool enable, float constant = 0.0f, float clamp = 0.0f, float slope = 0.0f) {
			ASSERT_FEATURE(clamp != 0.0f, device, DepthBiasClamp);

			rasterizer.depthBiasEnable = enable;
			rasterizer.depthBiasConstantFactor = constant;
			rasterizer.depthBiasClamp = clamp;
			rasterizer.depthBiasSlopeFactor = slope;
		}

		void setPolygonMode(VkPolygonMode mode) {
			ASSERT_FEATURE(mode != VK_POLYGON_MODE_FILL, device, FillModeNonSolid);
			rasterizer.polygonMode = mode;
		}

		void setLineWidth(float width) {
			ASSERT_FEATURE(width > 1.0f, device, WideLines);
			rasterizer.lineWidth = width;
		}

		void setCulling(bool enable, VkFrontFace face = VK_FRONT_FACE_CLOCKWISE, VkCullModeFlags mode = VK_CULL_MODE_BACK_BIT) {
			rasterizer.cullMode = enable ? mode : VK_CULL_MODE_NONE;
			rasterizer.frontFace = face;
		}

	// multisampling configuration

		// TODO

	// depth configuration

		void setBlendConstants(float r, float g, float b, float a) {
			blending.blendConstants[0] = r;
			blending.blendConstants[1] = g;
			blending.blendConstants[2] = b;
			blending.blendConstants[3] = a;
		}

		// finalColor = finalColor & writeMask;
		void setWriteMask(VkColorComponentFlags flags) {
			attachment.colorWriteMask = flags;
		}

		// finalColor.rgb = (src * newColor.rgb) <op> (dst * oldColor.rgb);
		void setBlendColorFunc(VkBlendFactor src, VkBlendOp op, VkBlendFactor dst) {
			attachment.srcColorBlendFactor = src;
			attachment.dstColorBlendFactor = dst;
			attachment.colorBlendOp = op;
		}

		// finalColor.a = (src * newColor.a) <op> (dst * oldColor.a);
		void setBlendAlphaFunc(VkBlendFactor src, VkBlendOp op, VkBlendFactor dst) {
			attachment.srcColorBlendFactor = src;
			attachment.dstColorBlendFactor = dst;
			attachment.colorBlendOp = op;
		}

		void setBlendBitwiseFunc(VkLogicOp op) {
			blending.logicOp = op;
		}

		void setBlendMode(GlphBlendMode mode) {
			if (mode == GLPH_BLEND_DISABLED) {
				blending.logicOpEnable = false;
				attachment.blendEnable = false;
			}

			if (mode == GLPH_BLEND_ENABLED) {
				blending.logicOpEnable = false;
				attachment.blendEnable = true;
			}

			if (mode == GLPH_BLEND_BITWISE) {
				ASSERT_FEATURE(true, device, LogicOp);
				blending.logicOpEnable = true;
				attachment.blendEnable = false;
			}
		}

	// renderpass configuration

		void setRenderPass(RenderPass& render_pass, uint32_t index = 0) {
			pass = render_pass.vk_pass;
			subpass = (int) index;
		}

	// shader stage configuration

		template<typename... Shaders>
		void setShaders(Shaders&... shaders) {
			stages = { (shaders.getStageConfig())... };
		}

	public:

		GraphicsPipeline build() {

			if (subpass == -1) {
				throw std::runtime_error("vkCreatePipelineLayout: Render pass needs to be specified!");
			}

			// update vector pointers in structures
			finalize();

			// first create the pipeline layout, it will be bundled with the pipeline
			VkPipelineLayout pipeline_layout;

			if (vkCreatePipelineLayout(device.vk_device, &layout, nullptr, &pipeline_layout) != VK_SUCCESS) {
				throw std::runtime_error("vkCreatePipelineLayout: Failed to create graphics pipeline layout!");
			}

			VkGraphicsPipelineCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			create_info.stageCount = (uint32_t) stages.size();
			create_info.pStages = stages.data();

			create_info.pVertexInputState = &input;
			create_info.pInputAssemblyState = &assembly;
			create_info.pViewportState = &view;
			create_info.pRasterizationState = &rasterizer;
			create_info.pMultisampleState = &multisampling;
			create_info.pDepthStencilState = nullptr; // optional
			create_info.pColorBlendState = &blending;
			create_info.pDynamicState = &dynamic;
			create_info.layout = pipeline_layout;

			create_info.renderPass = pass;
			create_info.subpass = subpass;

			// optional inheritence
			create_info.basePipelineHandle = VK_NULL_HANDLE;
			create_info.basePipelineIndex = -1;

			VkPipeline pipeline;

			if (vkCreateGraphicsPipelines(device.vk_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline) != VK_SUCCESS) {
				throw std::runtime_error("vkCreateGraphicsPipelines: Failed to create graphics pipeline!");
			}

			return {pipeline, pipeline_layout, device.vk_device};

		}

};
