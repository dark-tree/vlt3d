#pragma once

#include "external.hpp"
#include "util/format.hpp"

class BindingBuilder {

	private:
	
		uint32_t offset;
		VkVertexInputBindingDescription& parent;
		std::vector<VkVertexInputAttributeDescription>& attributes;
		
	public:
	
		BindingBuilder(VkVertexInputBindingDescription& parent, std::vector<VkVertexInputAttributeDescription>& attributes)
		: offset(0), parent(parent), attributes(attributes) {}
	
		BindingBuilder& addAttribute(uint32_t location, VkFormat format) {
		
			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkVertexInputAttributeDescription.html
			VkVertexInputAttributeDescription description {};
			description.location = location;
			description.binding = parent.binding;
			description.format = format;
			description.offset = offset;

			offset += getFormatInfo(format).size;
			attributes.push_back(description);
			return *this;
		}
		
		void done() {
			parent.stride = offset;
		}

};
