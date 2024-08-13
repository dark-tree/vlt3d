
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
		READONLY VkImageTiling vk_tiling = VK_IMAGE_TILING_OPTIMAL;
		READONLY VkFilter vk_filter = VK_FILTER_LINEAR;
		READONLY VkSamplerAddressMode vk_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		READONLY VkBorderColor vk_border = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		READONLY Image image;
		READONLY ImageView view;
		READONLY ImageSampler sampler;

	public:

		Attachment() = default;

		void allocate(Device& device, VkExtent2D extent, Allocator& allocator) {
			ImageInfo info {extent.width, extent.height, vk_format, vk_usage};
			info.required(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			info.tiling(vk_tiling);

			// close if it was already allocated
			close(device);

			allocated = true;
			image = allocator.allocateImage(info);
			view = image.getViewBuilder().build(device, vk_aspect);
			sampler = view.getSamplerBuilder().setFilter(vk_filter).setMode(vk_mode).setBorder(vk_border).build(device);
		}

		void close(Device& device) {
			if (allocated) {
				sampler.close(device);
				view.close(device);
				image.close();
				allocated = false;
			}
		}

};

class AttachmentImageBuilder {

	private:

		Attachment attachment;

	public:

		static AttachmentImageBuilder begin() {
			return {};
		}

	public:

		AttachmentImageBuilder& setFormat(VkFormat format) {
			attachment.vk_format = format;
			return *this;
		}

		AttachmentImageBuilder& setUsage(VkImageUsageFlags usage) {
			attachment.vk_usage = usage;
			return *this;
		}

		AttachmentImageBuilder& setAspect(VkImageAspectFlags aspect) {
			attachment.vk_aspect = aspect;
			return *this;
		}

		AttachmentImageBuilder& setTiling(VkImageTiling tiling) {
			attachment.vk_tiling = tiling;
			return *this;
		}

		AttachmentImageBuilder& setFilter(VkFilter filter) {
			attachment.vk_filter = filter;
			return *this;
		}

		AttachmentImageBuilder& setMode(VkSamplerAddressMode mode) {
			attachment.vk_mode = mode;
			return *this;
		}

		AttachmentImageBuilder& setBorder(VkBorderColor border) {
			attachment.vk_border = border;
			return *this;
		}

		AttachmentImageBuilder& setColorClearValue(float r, float g, float b, float a) {
			attachment.vk_clear.color = {.float32 = {r, g, b, a}};
			return *this;
		}

		AttachmentImageBuilder& setColorClearValue(int r, int g, int b, int a) {
			attachment.vk_clear.color = {.int32 = {r, g, b, a}};
			return *this;
		}

		AttachmentImageBuilder& setDepthClearValue(float depth, uint32_t stencil = 0) {
			attachment.vk_clear.depthStencil = {depth, stencil};
			return *this;
		}

		Attachment build() const {
			return attachment;
		}

};
