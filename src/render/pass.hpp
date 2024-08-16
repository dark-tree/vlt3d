
#pragma once

#include <utility>

#include "external.hpp"
#include "attachment.hpp"
#include "util/pyramid.hpp"
#include "setup/device.hpp"
#include "setup/debug.hpp"

class RenderPassBuilder;

template<typename T = RenderPassBuilder>
class AttachmentBuilder {

	private:

		T& builder;
		VkAttachmentDescription description {};

		friend class RenderPassBuilder;

		VkAttachmentDescription finalize() {
			return description;
		}

		AttachmentBuilder& input(VkAttachmentLoadOp color, VkAttachmentLoadOp stencil, VkImageLayout layout) {
			description.loadOp = color;
			description.stencilLoadOp = stencil;
			description.initialLayout = layout;
			return *this;
		}

		AttachmentBuilder& output(VkAttachmentStoreOp color, VkAttachmentStoreOp stencil, VkImageLayout layout) {
			description.storeOp = color;
			description.stencilStoreOp = stencil;
			description.finalLayout = layout;
			return *this;
		}

	public:

		AttachmentBuilder(T& builder, VkFormat format, VkSampleCountFlagBits samples)
		: builder(builder) {
			description.format = format;
			description.samples = samples;
		}

		/**
		 * Describes how the attachment data should be treated on load at the start of a render pass
		 *
		 * @param color describes what should happen to the color data
		 * @param stencil describes what should happen to the stencil data
		 * @param layout describes what layout should be used
		 */
		template <typename C, typename S>
		AttachmentBuilder& begin(AttachmentOp<ColorOp, C> color, AttachmentOp<StencilOp, S> stencil, VkImageLayout layout) {
			return input(color.load(), stencil.load(), layout);
		}

		template <typename C>
		AttachmentBuilder& begin(AttachmentOp<ColorOp, C> color, VkImageLayout layout) {
			return begin(color, StencilOp::IGNORE, layout);
		}

		/**
		 * Describes how the attachment data should be treated after the render pass
		 *
		 * @param color describes what should happen to the color data
		 * @param stencil describes what should happen to the stencil data
		 * @param layout describes what layout should be used
		 */
		template <typename C, typename S>
		AttachmentBuilder& end(AttachmentOp<ColorOp, C> color, AttachmentOp<StencilOp, S> stencil, VkImageLayout layout) {
			return output(color.store(), stencil.store(), layout);
		}

		template <typename C>
		AttachmentBuilder& end(AttachmentOp<ColorOp, C> color, VkImageLayout layout) {
			return end(color, StencilOp::IGNORE, layout);
		}

		Attachment::Ref next() {
			return builder.addAttachment(*this);
		}

};

template<typename T = RenderPassBuilder>
class DependencyBuilder {

	private:

		T& builder;
		VkSubpassDependency description {};

		friend class RenderPassBuilder;

		VkSubpassDependency finalize() {
			return description;
		}

	public:

		DependencyBuilder(T& builder, VkDependencyFlags flags)
		: builder(builder) {
			description.dependencyFlags = flags;
		}

		/**
		 * Specify the operation to wait for in the given stage of the given subpass
		 *
		 * @param subpass the index of the subpass in which is the stage and operation to wait for
		 * @param stage the stage in which is the operation to wait for
		 * @param access the operation to wait for, pass 0 to just wait for stage
		 */
		DependencyBuilder& input(uint32_t subpass, VkPipelineStageFlags stage, VkAccessFlags access) {
			description.srcSubpass = subpass;
			description.srcStageMask = stage;
			description.srcAccessMask = access;
			return *this;
		}

		/**
		 * Specify the operation of a stage of a subpass that will wait for this dependency
		 *
		 * @param subpass the index of the subpass in which is the stage and operation to wait for
		 * @param stage the stage in which is the operation to wait for
		 * @param access the operation to wait for, pass 0 to just wait for stage
		 */
		DependencyBuilder& output(uint32_t subpass, VkPipelineStageFlags stage, VkAccessFlags access) {
			description.dstSubpass = subpass;
			description.dstStageMask = stage;
			description.dstAccessMask = access;
			return *this;
		}

		T& next() {
			return builder.addDependency(*this);
		}

};

template<typename T = RenderPassBuilder>
class SubpassBuilder {

	private:

		T& builder;
		VkSubpassDescription description {};
		uint32_t attachment_count;
		Pyramid<uint32_t>& preserve;
		std::set<uint32_t> references;

		std::vector<VkAttachmentReference> inputs;
		std::vector<VkAttachmentReference> colors;
		std::vector<VkAttachmentReference> depths;
		std::vector<VkAttachmentReference> resolves;

		VkAttachmentReference getReference(uint32_t attachment, VkImageLayout layout) {

			if (attachment >= attachment_count) {
				throw Exception {"Attachment index " + std::to_string(attachment) + " out of bounds, only " + std::to_string(attachment_count) + " have been defined up to this point!"};
			}

			preserve.append(attachment);

			VkAttachmentReference reference {};
			reference.attachment = attachment;
			reference.layout = layout;

			return reference;

		}

		friend class RenderPassBuilder;

		bool shouldPreserve(uint32_t attachment) const {
			return !references.contains(attachment);
		}

		VkSubpassDescription finalize(const std::vector<uint32_t>& preserve, std::vector<int>& subpass_attachments) {

			uint32_t input_count = inputs.size();
			uint32_t color_count = colors.size();
			uint32_t depth_count = depths.size();
			uint32_t resolve_count = resolves.size();

			if (depth_count != 0 && resolve_count != 0) {
				if (depth_count != color_count) throw Exception {"Invalid number of depth attachments! Must be 0 or equal to the number of color attachments!"};
				if (resolve_count != color_count) throw Exception {"Invalid number of resolve attachments! Must be 0 or equal to the number of color attachments!"};
			}

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSubpassDescription.html
			description.inputAttachmentCount = input_count;
			description.pInputAttachments = inputs.data();
			description.colorAttachmentCount = color_count;
			description.pColorAttachments = colors.data();

			description.pResolveAttachments = resolves.data();
			description.pDepthStencilAttachment = depths.data();

			description.preserveAttachmentCount = (uint32_t) preserve.size();
			description.pPreserveAttachments = preserve.data();

			// this is here so that the renderpass can retain the information about
			// how many attachments were there for each subpass - this is then used during pipeline
			// creation to setup blending for each attachment
			subpass_attachments.push_back(color_count);

			return description;

		}

	public:

		SubpassBuilder(T& builder, VkPipelineBindPoint bind_point, uint32_t attachment_count, Pyramid<uint32_t>& preserve)
		: builder(builder), attachment_count(attachment_count), preserve(preserve) {
			description.pipelineBindPoint = bind_point;
		}

		/// attachments that are read from a shader
		SubpassBuilder& addInput(Attachment::Ref attachment, VkImageLayout target_layout) {
			references.insert(attachment.index);
			inputs.push_back(getReference(attachment.index, target_layout));
			return *this;
		}

		/// attachment for color data
		SubpassBuilder& addOutput(Attachment::Ref attachment, VkImageLayout target_layout) {
			references.insert(attachment.index);
			colors.push_back(getReference(attachment.index, target_layout));
			return *this;
		}

		/// attachment for depth and stencil data
		SubpassBuilder& addDepth(Attachment::Ref attachment, VkImageLayout target_layout) {
			references.insert(attachment.index);
			depths.push_back(getReference(attachment.index, target_layout));
			return *this;
		}

		/// attachments used for multisampling color attachments
		SubpassBuilder& addResolve(Attachment::Ref attachment, VkImageLayout target_layout) {
			references.insert(attachment.index);
			resolves.push_back(getReference(attachment.index, target_layout));
			return *this;
		}

		T& next() {
			return builder.addSubpass(*this);
		}

};

class RenderPass {

	public:

		READONLY VkDevice vk_device;
		READONLY VkRenderPass vk_pass;
		READONLY std::vector<VkClearValue> values;
		READONLY std::vector<int> subpasses;

	public:

		RenderPass() = default;
		RenderPass(VkDevice vk_device, VkRenderPass vk_pass, std::vector<VkClearValue>& values, std::vector<int>& subpass_attachments)
		: vk_device(vk_device), vk_pass(vk_pass), values(values), subpasses(subpass_attachments) {
			values.shrink_to_fit();
		}

		void close() {
			vkDestroyRenderPass(vk_device, vk_pass, AllocatorCallbackFactory::named("RenderPass"));
		}

		int getAttachmentCount(int subpass) const {
			return subpasses[subpass];
		}

		int getSubpassCount() const {
			return subpasses.size();
		}

};

class RenderPassBuilder {

	private:

		std::vector<VkClearValue> values;
		std::vector<AttachmentBuilder<>> attachments;
		std::vector<SubpassBuilder<>> subpasses;
		std::vector<DependencyBuilder<>> dependencies;

		Pyramid<uint32_t> preserve;

	public:

		Attachment::Ref addAttachment(AttachmentBuilder<>& builder) {
			attachments.push_back(builder);
			return attachments.size() - 1;
		}

		/**
		 * Adds and makes usable an attachment for subpasses in the render pass
		 */
		AttachmentBuilder<> addAttachment(VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) {
			return {*this, format, samples};
		}

		AttachmentBuilder<> addAttachment(const Attachment& attachment, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT) {
			values.push_back(attachment.vk_clear);
			return addAttachment(attachment.vk_format, samples);
		}

	public:

		RenderPassBuilder& addSubpass(SubpassBuilder<>& builder) {
			subpasses.push_back(builder);
			return *this;
		}

		/**
		 * Adds a render pass sub-stage, subpasses are executed in order
		 */
		SubpassBuilder<> addSubpass(VkPipelineBindPoint bind_point) {
			preserve.push();
			return {*this, bind_point, (uint32_t) attachments.size(), preserve};
		}

	public:

		RenderPassBuilder& addDependency(DependencyBuilder<>& builder) {
			dependencies.push_back(builder);
			return *this;
		}

		/**
		 * Add a data dependency to a subpass
		 */
		DependencyBuilder<> addDependency(VkDependencyFlags flags = 0) {
			return {*this, flags};
		}

	public:

		RenderPass build(Device& device, const char* name = nullptr) {

			std::vector<VkAttachmentDescription> attachment_descriptions;
			std::vector<VkSubpassDescription> subpass_descriptions;
			std::vector<VkSubpassDependency> dependency_descriptions;
			std::vector<std::vector<uint32_t>> preserve_indices;
			std::vector<int> subpass_attachments;

			auto view = preserve.view();

			attachment_descriptions.reserve(attachments.size());
			subpass_descriptions.reserve(subpasses.size());
			dependency_descriptions.reserve(dependencies.size());

			for (auto& attachment : attachments) {
				attachment_descriptions.push_back(attachment.finalize());
			}

			for (auto& dependency : dependencies) {
				dependency_descriptions.push_back(dependency.finalize());
			}

			for (auto& subpass : subpasses) {
				view.up();
				auto set = view.collect();

				preserve_indices.emplace_back();

				for (uint32_t attachment : set) {
					if (subpass.shouldPreserve(attachment)) {
						preserve_indices.back().push_back(attachment);
					}
				}

				subpass_descriptions.push_back(subpass.finalize(preserve_indices.back(), subpass_attachments));
			}

			VkRenderPassCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

			create_info.attachmentCount = (uint32_t) attachment_descriptions.size();
			create_info.pAttachments = attachment_descriptions.data();
			create_info.subpassCount = (uint32_t) subpass_descriptions.size();
			create_info.pSubpasses = subpass_descriptions.data();
			create_info.dependencyCount = (uint32_t) dependency_descriptions.size();
			create_info.pDependencies = dependency_descriptions.data();

			VkRenderPass render_pass;
			if (vkCreateRenderPass(device.vk_device, &create_info, AllocatorCallbackFactory::named("RenderPass"), &render_pass) != VK_SUCCESS) {
				throw Exception {"Failed to create render pass!"};
			}

			VulkanDebug::name(device.vk_device, VK_OBJECT_TYPE_RENDER_PASS, render_pass, name);
			return {device.vk_device, render_pass, values, subpass_attachments};

		}

};
