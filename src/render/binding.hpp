#pragma once

#include "external.hpp"
#include "util/format.hpp"

class BindingLayout {

	private:

		uint32_t stride;
		std::vector<VkVertexInputAttributeDescription> attributes;
		VkVertexInputRate vk_rate;

	public:

		BindingLayout(uint32_t stride, std::vector<VkVertexInputAttributeDescription> attributes, VkVertexInputRate rate)
		: stride(stride), attributes(attributes), vk_rate(rate) {}

		/**
		 * Returns a copy of the attribute vector
		 */
		std::vector<VkVertexInputAttributeDescription> getAttributes() const {
			return attributes;
		}

		/**
		 * Returns the stride of the binding
		 */
		uint32_t getStride() const {
			return stride;
		}

		/**
		 * Returns the input interval
		 */
		VkVertexInputRate getRate() const {
			return vk_rate;
		}

};

class BindingLayoutBuilder {

	private:

		uint32_t offset;
		std::vector<VkVertexInputAttributeDescription> attributes;
		VkVertexInputRate vk_rate;

	public:

		BindingLayoutBuilder(VkVertexInputRate rate)
		: offset(0), attributes(), vk_rate(rate) {}

		inline static BindingLayoutBuilder begin(VkVertexInputRate rate = VK_VERTEX_INPUT_RATE_VERTEX) {
			return {rate};
		}

	public:

		BindingLayoutBuilder& attribute(uint32_t location, VkFormat format) {

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVertexInputAttributeDescription.html
			VkVertexInputAttributeDescription description {};
			description.location = location;
			description.format = format;
			description.offset = offset;

			offset += getFormatInfo(format).size;
			attributes.push_back(description);
			return *this;
		}

		BindingLayout done() {
			return {offset, attributes, vk_rate};
		}

};
