#pragma once

#include "external.hpp"
#include "shader/kind.hpp"

class PushConstant {

	public:

		READONLY VkShaderStageFlags flags;
		READONLY uint32_t offset;
		READONLY uint32_t size;

	public:

		PushConstant() = default;
		PushConstant(VkPushConstantRange range);

};

class PushConstantLayout {

	public:

		READONLY std::vector<VkPushConstantRange> constants;

	public:

		PushConstantLayout() = default;
		PushConstantLayout(const std::vector<VkPushConstantRange>& constants);

		const VkPushConstantRange* data() const;
		uint32_t size() const;

};

class PushConstantLayoutBuilder {

	private:

		uint32_t offset = 0;
		std::vector<VkPushConstantRange> vk_constants;

	public:

		PushConstantLayoutBuilder& add(Kind kind, uint32_t bytes, PushConstant* constant);
		PushConstantLayoutBuilder& add(VkShaderStageFlags stage, uint32_t bytes, PushConstant* constant);
		PushConstantLayout done() const;

		inline static PushConstantLayoutBuilder begin() {
			return {};
		}

};
