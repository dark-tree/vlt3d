#pragma once

#include "external.hpp"

/**
 * See: https://docs.vulkan.org/samples/latest/samples/extensions/debug_utils/README.html
 */
class VulkanDebug {

	private:

		/// Constructs the struct used by insert() and begin()
		static VkDebugUtilsLabelEXT getDebugLabel(const char* debug_name, glm::vec3 color);

		/// Converts the VkObjectType enum into a human readable string
		static const char* getObjectName(VkObjectType type);

	public:

		/**
		 * Adds a debug name visible in tools like RenderDoc to any Vulkan object
		 */
		static void name(VkDevice device, VkObjectType type, void* object, const char* debug_name);

		/**
		 * Adds a debug label (name + color) into the command buffer
		 */
		static void insert(VkCommandBuffer vk_buffer, const char* debug_name, glm::vec3 color);

		/**
		 * Begins a named debug block (name + color), all commands until matching end() will appear inside
		 */
		static void begin(VkCommandBuffer vk_buffer, const char* debug_name, glm::vec3 color);

		/**
		 * End the previously began named debug block
		 */
		static void end(VkCommandBuffer vk_buffer);

};
