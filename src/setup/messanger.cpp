
#include "messanger.hpp"
#include "util/exception.hpp"
#include "setup/callback.hpp"

/*
 * DebugMessenger
 */

void DebugMessenger::set(VkInstance vk_instance, VkDebugUtilsMessengerEXT vk_messenger) {
	this->vk_instance = vk_instance;
	this->vk_messenger = vk_messenger;
}

VkBool32 DebugMessenger::onMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity, const VkDebugUtilsMessengerCallbackDataEXT* data) {
	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		logger::error(PREFIX, " ", data->pMessage);
		error_count ++;
		return VK_FALSE;
	}

	if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		logger::warn(PREFIX, " ", data->pMessage);
		warning_count ++;
		return VK_FALSE;
	}

	return VK_FALSE;
}

DebugMessenger::~DebugMessenger() {
	if (vk_messenger) {
		vkDestroyDebugUtilsMessengerEXT(vk_instance, vk_messenger, AllocatorCallbackFactory::named("DebugMessenger"));
		vk_messenger = nullptr;
	}
}

int DebugMessenger::getErrorCount() const {
	return error_count;
}

/*
 * DebugMessengerConfig
 */

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerConfig::MessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void* user) {
	return ((DebugMessenger*) user)->onMessage(severity, data);
}

void DebugMessengerConfig::enable() {

	if (messenger) {
		return;
	}

	messenger = new DebugMessenger();

	// configure the messenger, this will later be used during instance creation and right after it
	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = MessageCallback;
	create_info.pUserData = messenger;

}

DebugMessenger* DebugMessengerConfig::attach(VkInstance vk_instance) {

	if (messenger) {
		VkDebugUtilsMessengerEXT vk_messenger;
		if (vkCreateDebugUtilsMessengerEXT(vk_instance, &create_info, AllocatorCallbackFactory::named("DebugMessenger"), &vk_messenger) != VK_SUCCESS) {
			throw Exception {"Failed to create debug messenger!"};
		}

		messenger->set(vk_instance, vk_messenger);
		return messenger;
	}

	return nullptr;
}

bool DebugMessengerConfig::isEnabled() const {
	return messenger;
}

VkDebugUtilsMessengerCreateInfoEXT* DebugMessengerConfig::getConfigPointer() {
	return &create_info;
}
