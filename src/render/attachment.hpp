
#pragma once

#include "external.hpp"
#include "view.hpp"
#include "buffer/allocator.hpp"

struct AttachmentOpType {

	struct Load {
		const VkAttachmentLoadOp vk_load;

		constexpr explicit Load(VkAttachmentLoadOp vk_load)
		: vk_load(vk_load) {}
	};

	struct Store {
		const VkAttachmentStoreOp vk_store;

		constexpr explicit Store(VkAttachmentStoreOp vk_store)
		: vk_store(vk_store) {}
	};

	struct Both {
		const VkAttachmentLoadOp vk_load;
		const VkAttachmentStoreOp vk_store;

		constexpr explicit Both(VkAttachmentLoadOp vk_load, VkAttachmentStoreOp vk_store)
		: vk_load(vk_load), vk_store(vk_store) {}
	};

};

template <typename T, typename S>
struct AttachmentOp {

	private:

		const S values;

	public:

		template <typename ...Args>
		constexpr explicit AttachmentOp(Args... args)
		: values(args...) {}

		template <typename R = VkAttachmentLoadOp> requires requires {values.vk_load;}
		constexpr auto load() const -> R {
			return values.vk_load;
		}

		template <typename R = VkAttachmentStoreOp> requires requires {values.vk_store;}
		constexpr auto store() const -> R {
			return values.vk_store;
		}

};

struct ColorOp {

	static constexpr AttachmentOp<ColorOp, AttachmentOpType::Load> CLEAR {VK_ATTACHMENT_LOAD_OP_CLEAR};
	static constexpr AttachmentOp<ColorOp, AttachmentOpType::Load> LOAD {VK_ATTACHMENT_LOAD_OP_LOAD};
	static constexpr AttachmentOp<ColorOp, AttachmentOpType::Store> STORE {VK_ATTACHMENT_STORE_OP_STORE};
	static constexpr AttachmentOp<ColorOp, AttachmentOpType::Both> IGNORE {VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE};

};

struct StencilOp {

	static constexpr AttachmentOp<StencilOp, AttachmentOpType::Load> CLEAR {VK_ATTACHMENT_LOAD_OP_CLEAR};
	static constexpr AttachmentOp<StencilOp, AttachmentOpType::Load> LOAD {VK_ATTACHMENT_LOAD_OP_LOAD};
	static constexpr AttachmentOp<StencilOp, AttachmentOpType::Store> STORE {VK_ATTACHMENT_STORE_OP_STORE};
	static constexpr AttachmentOp<StencilOp, AttachmentOpType::Both> IGNORE {VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE};

};

/**
 * Abstraction over the "Image-View-Sampler" triple in the context of full-screen images used
 * in shaders as outputs (or inputs) such as the depth buffer or the various images used in deferred rendering
 */
class Attachment {

	private:

		bool allocated = false;

	public:

		struct Ref {
			const uint32_t index;

			Ref(int index)
			: index(index) {}
		};

		READONLY VkFormat vk_format;
		READONLY VkImageUsageFlags vk_usage;
		READONLY VkImageAspectFlags vk_aspect;
		READONLY VkClearValue vk_clear;
		READONLY VkFilter vk_filter = VK_FILTER_LINEAR;
		READONLY VkSamplerAddressMode vk_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		READONLY VkBorderColor vk_border = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		READONLY Image image;
		READONLY ImageView view;
		READONLY ImageSampler sampler;
		READONLY const char* debug_name;

	public:

		Attachment() = default;

		void allocate(Device& device, VkExtent2D extent, Allocator& allocator);

		void close(Device& device);

};

class AttachmentImageBuilder {

	private:

		Attachment attachment;

	public:

		static AttachmentImageBuilder begin();

	public:

		AttachmentImageBuilder& setFormat(VkFormat format);
		AttachmentImageBuilder& setUsage(VkImageUsageFlags usage);
		AttachmentImageBuilder& setAspect(VkImageAspectFlags aspect);
		AttachmentImageBuilder& setFilter(VkFilter filter);
		AttachmentImageBuilder& setMode(VkSamplerAddressMode mode);
		AttachmentImageBuilder& setBorder(VkBorderColor border);
		AttachmentImageBuilder& setColorClearValue(float r, float g, float b, float a);
		AttachmentImageBuilder& setColorClearValue(int r, int g, int b, int a);
		AttachmentImageBuilder& setDepthClearValue(float depth, uint32_t stencil = 0);
		AttachmentImageBuilder& setDebugName(const char* name);

		Attachment build() const;

};
