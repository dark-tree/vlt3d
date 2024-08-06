
#include "external.hpp"
#include "util/logger.hpp"

static std::vector<std::pair<PFN_vkVoidFunction*, const char*>> instance_proxies;
static std::vector<std::pair<PFN_vkVoidFunction*, const char*>> device_proxies;

void initProxyInstance(VkInstance& instance) {
	for (auto& pair : instance_proxies) {
		PFN_vkVoidFunction function = vkGetInstanceProcAddr(instance, pair.second);

		if (function == nullptr) {
			logger::warn("Instance function '", pair.second, "' failed to load!");
		}

		*pair.first = function;
	}
}

void initProxyDevice(VkDevice& device) {
	for (auto& pair : device_proxies) {
		PFN_vkVoidFunction function = vkGetDeviceProcAddr(device, pair.second);

		if (function == nullptr) {
			logger::warn("Device function '", pair.second, "' failed to load!");
		}

		*pair.first = function;
	}
}

// generated with a python script and pasted here

// Proxy for function vkCreateDebugUtilsMessengerEXT
static PFN_vkCreateDebugUtilsMessengerEXT proxy_vkCreateDebugUtilsMessengerEXT = nullptr;
[[maybe_unused]] static bool symbol_vkCreateDebugUtilsMessengerEXT = [] () {
	instance_proxies.emplace_back((PFN_vkVoidFunction*) &proxy_vkCreateDebugUtilsMessengerEXT, "vkCreateDebugUtilsMessengerEXT");
	return true;
}();
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
	return proxy_vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

// Proxy for function vkDestroyDebugUtilsMessengerEXT
static PFN_vkDestroyDebugUtilsMessengerEXT proxy_vkDestroyDebugUtilsMessengerEXT = nullptr;
[[maybe_unused]] static bool symbol_vkDestroyDebugUtilsMessengerEXT = [] () {
	instance_proxies.emplace_back((PFN_vkVoidFunction*) &proxy_vkDestroyDebugUtilsMessengerEXT, "vkDestroyDebugUtilsMessengerEXT");
	return true;
}();
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
	return proxy_vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}


