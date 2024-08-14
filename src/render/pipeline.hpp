
#pragma once

#include "external.hpp"
#include "util/format.hpp"
#include "pass.hpp"
#include "setup/features.hpp"
#include "binding.hpp"
#include "descriptor/layout.hpp"
#include "util/timer.hpp"
#include "shader/module.hpp"
#include "descriptor/push.hpp"

#define ASSERT_FEATURE(test, device, feature) if ((test) && !device.features.has##feature ()) { throw Exception {"Feature '" #feature "' not enabled on this device!"}; }

enum struct BlendMode {
	BITWISE = 1,
	ENABLED = 2,
	DISABLED = 3
};

class GraphicsPipeline {

	public:

		READONLY VkPipeline vk_pipeline;
		READONLY VkPipelineLayout vk_layout;
		READONLY VkDevice vk_device;

	public:

		GraphicsPipeline() = default;
		GraphicsPipeline(VkPipeline vk_pipeline, VkPipelineLayout vk_layout, VkDevice vk_device)
		: vk_pipeline(vk_pipeline), vk_layout(vk_layout), vk_device(vk_device) {}

		void close() {
			vkDestroyPipeline(vk_device, vk_pipeline, AllocatorCallbackFactory::named("Pipeline"));
			vkDestroyPipelineLayout(vk_device, vk_layout, AllocatorCallbackFactory::named("PipelineLayout"));
		}

};

class GraphicsPipelineBuilder {

	private:

		std::vector<VkDescriptorSetLayout> sets;
		std::vector<VkDynamicState> dynamics;
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;
		std::vector<VkPipelineColorBlendAttachmentState> attachments;

		std::vector<ShaderModule> stages;

		VkPipelineViewportStateCreateInfo view {};
		VkPipelineDynamicStateCreateInfo dynamic {};
		VkPipelineVertexInputStateCreateInfo input {};
		VkPipelineLayoutCreateInfo layout {};
		VkPipelineInputAssemblyStateCreateInfo assembly {};
		VkPipelineRasterizationStateCreateInfo rasterizer {};
		VkPipelineMultisampleStateCreateInfo multisampling {};
		VkPipelineColorBlendAttachmentState attachment {};
		VkPipelineColorBlendStateCreateInfo blending {};
		VkPipelineDepthStencilStateCreateInfo depth {};
		PushConstantLayout push {};

		VkViewport vk_viewport {};
		VkRect2D vk_scissor {};
		VkRenderPass vk_pass;
		int subpass = -1;
		Device& device;

		void finalize() {

			input.vertexBindingDescriptionCount = (uint32_t) bindings.size();
			input.pVertexBindingDescriptions = bindings.data();

			input.vertexAttributeDescriptionCount = (uint32_t) attributes.size();
			input.pVertexAttributeDescriptions = attributes.data();

			dynamic.dynamicStateCount = (uint32_t) dynamics.size();
			dynamic.pDynamicStates = dynamics.data();

			layout.setLayoutCount = sets.size();
			layout.pSetLayouts = sets.data();
			layout.pushConstantRangeCount = push.size();
			layout.pPushConstantRanges = push.data();

			view.viewportCount = 1;
			view.pViewports = &vk_viewport;
			view.scissorCount = 1;
			view.pScissors = &vk_scissor;

			// technically blending is per-pipeline and per-attachment but we limit it to per-pipeline only here
			for (int i = 0; i < (int) blending.attachmentCount; i ++) {
				attachments.push_back(attachment);
			}

			blending.pAttachments = attachments.data();

		}

	public:

		GraphicsPipelineBuilder(Device& device)
		: device(device) {

			depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

			view.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

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

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineInputAssemblyStateCreateInfo.html
			assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			withPrimitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineRasterizationStateCreateInfo.html
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			withDepthClamp(false);
			withDepthBias(false);
			withPolygonMode(VK_POLYGON_MODE_FILL);
			withLineWidth(1.0f);
			withCulling(false);

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineMultisampleStateCreateInfo.html
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.flags = 0;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			// TODO

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineColorBlendStateCreateInfo.html
			blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			blending.attachmentCount = 0;
			withBlendConstants(0.0f, 0.0f, 0.0f, 0.0f);
			withBlendWriteMask(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
			withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
			withBlendAlphaFunc(VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ZERO);
			withBlendBitwiseFunc(VK_LOGIC_OP_COPY);
			withBlendMode(BlendMode::DISABLED);

		}

		inline static GraphicsPipelineBuilder of(Device& device) {
			return {device};
		}

	// dynamic configuration

		template <typename... DynamicState>
		GraphicsPipelineBuilder& withDynamics(DynamicState... states) {
			dynamics = { states... };
			return *this;
		}

	// view configuration

		GraphicsPipelineBuilder& withViewport(int x, int y, uint32_t width, uint32_t height, float min_depth = 0.0f, float max_depth = 1.0f) {
			vk_viewport.x = (float) x;
			vk_viewport.y = (float) y;
			vk_viewport.width = (float) width;
			vk_viewport.height = (float) height;
			vk_viewport.minDepth = min_depth;
			vk_viewport.maxDepth = max_depth;
			return *this;
		}

		GraphicsPipelineBuilder& withScissors(int x, int y, uint32_t width, uint32_t height) {
			vk_scissor.offset = {x, y};
			vk_scissor.extent = {width, height};
			return *this;
		}

	// vertex configuration

		GraphicsPipelineBuilder& withBindingLayout(BindingLayout& layout) {
			const uint32_t binding = (uint32_t) bindings.size();

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVertexInputBindingDescription.html
			VkVertexInputBindingDescription description {};
			description.binding = binding;
			description.stride = layout.getStride();
			description.inputRate = layout.getRate();

			bindings.push_back(description);

			for (VkVertexInputAttributeDescription& attribute : layout.getAttributes()) {
				attribute.binding = binding;
				this->attributes.push_back(attribute);
			}

			return *this;
		}

	// layout configuration

		GraphicsPipelineBuilder& withPushConstantLayout(const PushConstantLayout& push) {
			this->push = push;
			return *this;
		}

		GraphicsPipelineBuilder& withDescriptorSetLayout(DescriptorSetLayout layout) {
			sets.push_back(layout.vk_layout);
			return *this;
		}

	// assembly configuration

		GraphicsPipelineBuilder& withPrimitive(VkPrimitiveTopology topology, bool enable_restart = false) {
			assembly.topology = topology;
			assembly.primitiveRestartEnable = enable_restart;
			return *this;
		}

	// rasterizer configuration

		GraphicsPipelineBuilder& withDepthClamp(bool enable) {
			ASSERT_FEATURE(enable, device, DepthClamp);
			rasterizer.depthClampEnable = enable;
			return *this;
		}

		GraphicsPipelineBuilder& withDepthBias(bool enable, float constant = 0.0f, float clamp = 0.0f, float slope = 0.0f) {
			ASSERT_FEATURE(clamp != 0.0f, device, DepthBiasClamp);
			rasterizer.depthBiasEnable = enable;
			rasterizer.depthBiasConstantFactor = constant;
			rasterizer.depthBiasClamp = clamp;
			rasterizer.depthBiasSlopeFactor = slope;
			return *this;
		}

		GraphicsPipelineBuilder& withPolygonMode(VkPolygonMode mode) {
			ASSERT_FEATURE(mode != VK_POLYGON_MODE_FILL, device, FillModeNonSolid);
			rasterizer.polygonMode = mode;
			return *this;
		}

		GraphicsPipelineBuilder& withLineWidth(float width) {
			ASSERT_FEATURE(width > 1.0f, device, WideLines);
			rasterizer.lineWidth = width;
			return *this;
		}

		GraphicsPipelineBuilder& withCulling(bool enable, VkFrontFace face = VK_FRONT_FACE_CLOCKWISE, VkCullModeFlags mode = VK_CULL_MODE_BACK_BIT) {
			rasterizer.cullMode = enable ? mode : VK_CULL_MODE_NONE;
			rasterizer.frontFace = face;
			return *this;
		}

	// multisampling configuration

		// TODO

	// depth configuration

		GraphicsPipelineBuilder& withDepthBound(float lower, float upper) {
			depth.depthBoundsTestEnable = true;
			depth.minDepthBounds = lower;
			depth.maxDepthBounds = upper;
			return *this;
		}

		GraphicsPipelineBuilder& withDepthTest(VkCompareOp function, bool read, bool write) {
			depth.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depth.depthCompareOp = function;
			depth.depthTestEnable = read;
			depth.depthWriteEnable = write;
			return *this;
		}

	// blend configuration

		GraphicsPipelineBuilder& withBlendConstants(float r, float g, float b, float a) {
			blending.blendConstants[0] = r;
			blending.blendConstants[1] = g;
			blending.blendConstants[2] = b;
			blending.blendConstants[3] = a;
			return *this;
		}

		// finalColor = finalColor & writeMask;
		GraphicsPipelineBuilder& withBlendWriteMask(VkColorComponentFlags flags) {
			attachment.colorWriteMask = flags;
			return *this;

		}

		// finalColor.rgb = (src * newColor.rgb) <op> (dst * oldColor.rgb);
		GraphicsPipelineBuilder& withBlendColorFunc(VkBlendFactor src, VkBlendOp op, VkBlendFactor dst) {
			attachment.srcColorBlendFactor = src;
			attachment.dstColorBlendFactor = dst;
			attachment.colorBlendOp = op;
			return *this;
		}

		// finalColor.a = (<src> * newColor.a) <op> (<dst> * oldColor.a);
		GraphicsPipelineBuilder& withBlendAlphaFunc(VkBlendFactor src, VkBlendOp op, VkBlendFactor dst) {
			attachment.srcAlphaBlendFactor = src;
			attachment.dstAlphaBlendFactor = dst;
			attachment.colorBlendOp = op;
			return *this;
		}

		GraphicsPipelineBuilder& withBlendBitwiseFunc(VkLogicOp op) {
			blending.logicOp = op;
			return *this;
		}

		GraphicsPipelineBuilder& withBlendMode(BlendMode mode) {
			if (mode == BlendMode::DISABLED) {
				blending.logicOpEnable = false;
				attachment.blendEnable = false;
			}

			if (mode == BlendMode::ENABLED) {
				blending.logicOpEnable = false;
				attachment.blendEnable = true;
			}

			if (mode == BlendMode::BITWISE) {
				ASSERT_FEATURE(true, device, LogicOp);
				blending.logicOpEnable = true;
				attachment.blendEnable = false;
			}

			return *this;
		}

	// renderpass configuration

		GraphicsPipelineBuilder& withRenderPass(RenderPass& render_pass, int index = 0) {
			const int count = render_pass.getSubpassCount();

			if (count <= index) {
				throw Exception {"Specified render pass has " + std::to_string(count) + " subpasses but, subpass with index " + std::to_string(index) + " was requested"};
			}

			blending.attachmentCount = render_pass.getAttachmentCount(index);
			vk_pass = render_pass.vk_pass;
			subpass = index;
			return *this;
		}

	// shader stage configuration

		template<typename... Shaders>
		GraphicsPipelineBuilder& withShaders(Shaders... shaders) {
			stages = { shaders... };
			return *this;
		}

	public:

		GraphicsPipeline build() {

			if (subpass == -1) {
				throw Exception {"Render pass needs to be specified!"};
			}

			// update vector pointers in structures
			finalize();

			// first create the pipeline layout, it will be bundled with the pipeline
			VkPipelineLayout pipeline_layout;

			if (vkCreatePipelineLayout(device.vk_device, &layout, AllocatorCallbackFactory::named("PipelineLayout"), &pipeline_layout) != VK_SUCCESS) {
				throw Exception {"Failed to create graphics pipeline layout!"};
			}

			std::vector<VkPipelineShaderStageCreateInfo> shaders;

			for (ShaderModule& stage : stages) {
				shaders.push_back(stage.getStageConfig());
			}

			VkGraphicsPipelineCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			create_info.stageCount = (uint32_t) shaders.size();
			create_info.pStages = shaders.data();

			create_info.pVertexInputState = &input;
			create_info.pInputAssemblyState = &assembly;
			create_info.pViewportState = &view;
			create_info.pRasterizationState = &rasterizer;
			create_info.pMultisampleState = &multisampling;
			create_info.pDepthStencilState = &depth;
			create_info.pColorBlendState = &blending;
			create_info.pDynamicState = &dynamic;
			create_info.layout = pipeline_layout;

			create_info.renderPass = vk_pass;
			create_info.subpass = subpass;

			// optional inheritance
			create_info.basePipelineHandle = VK_NULL_HANDLE;
			create_info.basePipelineIndex = -1;

			// init noop stencil
			depth.stencilTestEnable = false;
			depth.front = {}; // Optional
			depth.back = {}; // Optional

			VkPipeline pipeline;

			if (vkCreateGraphicsPipelines(device.vk_device, VK_NULL_HANDLE, 1, &create_info, AllocatorCallbackFactory::named("Pipeline"), &pipeline) != VK_SUCCESS) {
				throw Exception {"Failed to create graphics pipeline!"};
			}

			return {pipeline, pipeline_layout, device.vk_device};

		}

};
