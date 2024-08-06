#pragma once

#include "external.hpp"
#include "util/exception.hpp"

class DebugMessenger {

	private:

		READONLY VkInstance instance;
		READONLY VkDebugUtilsMessengerEXT messenger;
		READONLY bool attached;

	public:

		DebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT& messenger)
		: instance(instance), messenger(messenger), attached(true) {}

		DebugMessenger()
		: messenger(nullptr), attached(false) {}

		void close() {
			if (attached) {
				vkDestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
				attached = false;
			}
		}
};

class DebugMessengerConfig {

	public:

		using CallbackFunction = std::function<VKAPI_ATTR VkBool32 VKAPI_CALL(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*)>;

		static VKAPI_ATTR VkBool32 VKAPI_CALL DefaultExt(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data) {
			return DefaultRaw(severity, type, data, nullptr);
		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL DefaultRaw(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void* user) {
			if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
				logger::error("[Vulkan] ", data->pMessage);
			} else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
				logger::warn("[Vulkan] ", data->pMessage);
			}

			return VK_FALSE;
		}

	private:

		static VKAPI_ATTR VkBool32 VKAPI_CALL WrapperFunction(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void* user) {
			return (*(CallbackFunction*)(user))(severity, type, data);
		}

		VkDebugUtilsMessengerCreateInfoEXT create_info {};
		bool enabled = false;

	public:

		void configure(const CallbackFunction& callback) {

			// yes we leak the function here
			configure(WrapperFunction, new CallbackFunction {callback});

		}

		void configure(PFN_vkDebugUtilsMessengerCallbackEXT callback, void* user_data) {

			// configure the messenger, this will later be used during instance creation and right after it
			create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			create_info.pfnUserCallback = callback;
			create_info.pUserData = user_data;

			enabled = true;
		}

	private:

		friend class InstanceBuilder;

		DebugMessenger attach(VkInstance instance) {

			if (enabled) {
				VkDebugUtilsMessengerEXT messenger;
				if (vkCreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &messenger) != VK_SUCCESS) {
					throw Exception {"Failed to create debug messenger"};
				}

				return {instance, messenger};
			}

			return {};
		}

		bool isEnabled() const {
			return enabled;
		}

		VkDebugUtilsMessengerCreateInfoEXT* getConfigPointer() {
			return &create_info;
		}

};
