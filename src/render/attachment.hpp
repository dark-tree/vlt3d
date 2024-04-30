
#pragma once

#include "external.hpp"

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