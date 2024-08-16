
#include "debug.hpp"

VkDebugUtilsLabelEXT VulkanDebug::getDebugLabel(const char* debug_name, float r, float g, float b) {
	VkDebugUtilsLabelEXT label {};
	label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	label.pNext = nullptr;
	label.pLabelName = debug_name;
	label.color[0] = r;
	label.color[1] = g;
	label.color[2] = b;
	label.color[3] = 1.0f;

	return label;
}

const char* VulkanDebug::getObjectName(VkObjectType type) {
	#if !defined(NDEBUG)
	if (type == VK_OBJECT_TYPE_INSTANCE) return "Instance";
	if (type == VK_OBJECT_TYPE_PHYSICAL_DEVICE) return "Physical Device";
	if (type == VK_OBJECT_TYPE_DEVICE) return "Device";
	if (type == VK_OBJECT_TYPE_QUEUE) return "Queue";
	if (type == VK_OBJECT_TYPE_SEMAPHORE) return "Semaphore";
	if (type == VK_OBJECT_TYPE_COMMAND_BUFFER) return "Command Buffer";
	if (type == VK_OBJECT_TYPE_FENCE) return "Fence";
	if (type == VK_OBJECT_TYPE_DEVICE_MEMORY) return "Device Memory";
	if (type == VK_OBJECT_TYPE_BUFFER) return "Buffer";
	if (type == VK_OBJECT_TYPE_IMAGE) return "Image";
	if (type == VK_OBJECT_TYPE_EVENT) return "Event";
	if (type == VK_OBJECT_TYPE_QUERY_POOL) return "Query Pool";
	if (type == VK_OBJECT_TYPE_BUFFER_VIEW) return "Buffer View";
	if (type == VK_OBJECT_TYPE_IMAGE_VIEW) return "Image View";
	if (type == VK_OBJECT_TYPE_SHADER_MODULE) return "Shader Module";
	if (type == VK_OBJECT_TYPE_PIPELINE_CACHE) return "Pipeline Cache";
	if (type == VK_OBJECT_TYPE_PIPELINE_LAYOUT) return "Pipeline Layout";
	if (type == VK_OBJECT_TYPE_RENDER_PASS) return "Render Pass";
	if (type == VK_OBJECT_TYPE_PIPELINE) return "Pipeline";
	if (type == VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT) return "Descriptor Set Layout";
	if (type == VK_OBJECT_TYPE_SAMPLER) return "Sampler";
	if (type == VK_OBJECT_TYPE_DESCRIPTOR_POOL) return "Descriptor Pool";
	if (type == VK_OBJECT_TYPE_DESCRIPTOR_SET) return "Descriptor Set";
	if (type == VK_OBJECT_TYPE_FRAMEBUFFER) return "Framebuffer";
	if (type == VK_OBJECT_TYPE_COMMAND_POOL) return "Command Pool";
	#endif

	return "Unknown";
}

void VulkanDebug::name(VkDevice vk_device, VkObjectType type, void* object, const char* debug_name) {
	#if !defined(NDEBUG)
	if (debug_name == nullptr) {
		return;
	}

	std::string fullname = getObjectName(type);
	fullname.push_back(' ');
	fullname.push_back('\'');
	fullname += debug_name;
	fullname.push_back('\'');

	VkDebugUtilsObjectNameInfoEXT name_info {};
	name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	name_info.objectType = type;
	name_info.objectHandle = (std::intptr_t) object;
	name_info.pObjectName = fullname.c_str();

	vkSetDebugUtilsObjectNameEXT(vk_device, &name_info);
	#endif
}

void VulkanDebug::insert(VkCommandBuffer vk_buffer, const char* debug_name, float r, float g, float b) {
	#if !defined(NDEBUG)
	VkDebugUtilsLabelEXT label = getDebugLabel(debug_name, r, g, b);
	vkCmdInsertDebugUtilsLabelEXT(vk_buffer, &label);
	#endif
}

void VulkanDebug::begin(VkCommandBuffer vk_buffer, const char* debug_name, float r, float g, float b) {
	#if !defined(NDEBUG)
	VkDebugUtilsLabelEXT label = getDebugLabel(debug_name, r, g, b);
	vkCmdBeginDebugUtilsLabelEXT(vk_buffer, &label);
	#endif
}

void VulkanDebug::end(VkCommandBuffer vk_buffer) {
	#if !defined(NDEBUG)
	vkCmdEndDebugUtilsLabelEXT(vk_buffer);
	#endif
}