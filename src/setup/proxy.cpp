
#include "external.hpp"
#include "util/logger.hpp"
#include "util/exception.hpp"

static std::unordered_map<std::string, PFN_vkVoidFunction*> instance_proxies;
static std::unordered_map<std::string, PFN_vkVoidFunction*> device_proxies;

void loadInstanceFunction(VkInstance& vk_instance, const char* function) {
	auto it = instance_proxies.find(function);

	if (it == instance_proxies.end()) {
		throw Exception {"Undefined instance function '" + std::string(function) + "' requested"};
	}

	PFN_vkVoidFunction address = vkGetInstanceProcAddr(vk_instance, it->first.c_str());

	if (address == nullptr) {
		logger::error("Instance function '", it->first, "' failed to load!");
	}

	*(it->second) = address;
	instance_proxies.erase(it);
}

void loadDeviceFunction(VkDevice& vk_device, const char* function) {
	auto it = device_proxies.find(function);

	if (it == device_proxies.end()) {
		throw Exception {"Undefined device function '" + std::string(function) + "' requested"};
	}

	PFN_vkVoidFunction address = vkGetDeviceProcAddr(vk_device, it->first.c_str());

	if (address == nullptr) {
		logger::error("Instance function '", it->first, "' failed to load!");
	}

	*(it->second) = address;
	device_proxies.erase(it);
}

// generated with a python script and pasted here

// Proxy for instance function vkCreateDebugUtilsMessengerEXT
static PFN_vkCreateDebugUtilsMessengerEXT proxy_vkCreateDebugUtilsMessengerEXT = nullptr;
[[maybe_unused]] static bool symbol_vkCreateDebugUtilsMessengerEXT = [] () {
	instance_proxies["vkCreateDebugUtilsMessengerEXT"] = ((PFN_vkVoidFunction*) &proxy_vkCreateDebugUtilsMessengerEXT);
	return true;
}();
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
	#if !defined(NDEBUG)
	if (!proxy_vkCreateDebugUtilsMessengerEXT) throw Exception {"Proxy instance function 'vkCreateDebugUtilsMessengerEXT' was called before being loaded"};
	#endif
	return proxy_vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

// Proxy for instance function vkDestroyDebugUtilsMessengerEXT
static PFN_vkDestroyDebugUtilsMessengerEXT proxy_vkDestroyDebugUtilsMessengerEXT = nullptr;
[[maybe_unused]] static bool symbol_vkDestroyDebugUtilsMessengerEXT = [] () {
	instance_proxies["vkDestroyDebugUtilsMessengerEXT"] = ((PFN_vkVoidFunction*) &proxy_vkDestroyDebugUtilsMessengerEXT);
	return true;
}();
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
	#if !defined(NDEBUG)
	if (!proxy_vkDestroyDebugUtilsMessengerEXT) throw Exception {"Proxy instance function 'vkDestroyDebugUtilsMessengerEXT' was called before being loaded"};
	#endif
	return proxy_vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

// Proxy for instance function vkGetPhysicalDeviceFeatures2KHR
static PFN_vkGetPhysicalDeviceFeatures2KHR proxy_vkGetPhysicalDeviceFeatures2KHR = nullptr;
[[maybe_unused]] static bool symbol_vkGetPhysicalDeviceFeatures2KHR = [] () {
	instance_proxies["vkGetPhysicalDeviceFeatures2KHR"] = ((PFN_vkVoidFunction*) &proxy_vkGetPhysicalDeviceFeatures2KHR);
	return true;
}();
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures2KHR( VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
	#if !defined(NDEBUG)
	if (!proxy_vkGetPhysicalDeviceFeatures2KHR) throw Exception {"Proxy instance function 'vkGetPhysicalDeviceFeatures2KHR' was called before being loaded"};
	#endif
	return proxy_vkGetPhysicalDeviceFeatures2KHR(physicalDevice, pFeatures);
}

// Proxy for device function vkSetDebugUtilsObjectNameEXT
static PFN_vkSetDebugUtilsObjectNameEXT proxy_vkSetDebugUtilsObjectNameEXT = nullptr;
[[maybe_unused]] static bool symbol_vkSetDebugUtilsObjectNameEXT = [] () {
	device_proxies["vkSetDebugUtilsObjectNameEXT"] = ((PFN_vkVoidFunction*) &proxy_vkSetDebugUtilsObjectNameEXT);
	return true;
}();
VKAPI_ATTR VkResult VKAPI_CALL vkSetDebugUtilsObjectNameEXT( VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) {
	#if !defined(NDEBUG)
	if (!proxy_vkSetDebugUtilsObjectNameEXT) throw Exception {"Proxy device function 'vkSetDebugUtilsObjectNameEXT' was called before being loaded"};
	#endif
	return proxy_vkSetDebugUtilsObjectNameEXT(device, pNameInfo);
}

// Proxy for device function vkCmdBeginDebugUtilsLabelEXT
static PFN_vkCmdBeginDebugUtilsLabelEXT proxy_vkCmdBeginDebugUtilsLabelEXT = nullptr;
[[maybe_unused]] static bool symbol_vkCmdBeginDebugUtilsLabelEXT = [] () {
	device_proxies["vkCmdBeginDebugUtilsLabelEXT"] = ((PFN_vkVoidFunction*) &proxy_vkCmdBeginDebugUtilsLabelEXT);
	return true;
}();
VKAPI_ATTR void VKAPI_CALL vkCmdBeginDebugUtilsLabelEXT( VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
	#if !defined(NDEBUG)
	if (!proxy_vkCmdBeginDebugUtilsLabelEXT) throw Exception {"Proxy device function 'vkCmdBeginDebugUtilsLabelEXT' was called before being loaded"};
	#endif
	return proxy_vkCmdBeginDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}

// Proxy for device function vkCmdEndDebugUtilsLabelEXT
static PFN_vkCmdEndDebugUtilsLabelEXT proxy_vkCmdEndDebugUtilsLabelEXT = nullptr;
[[maybe_unused]] static bool symbol_vkCmdEndDebugUtilsLabelEXT = [] () {
	device_proxies["vkCmdEndDebugUtilsLabelEXT"] = ((PFN_vkVoidFunction*) &proxy_vkCmdEndDebugUtilsLabelEXT);
	return true;
}();
VKAPI_ATTR void VKAPI_CALL vkCmdEndDebugUtilsLabelEXT( VkCommandBuffer commandBuffer) {
	#if !defined(NDEBUG)
	if (!proxy_vkCmdEndDebugUtilsLabelEXT) throw Exception {"Proxy device function 'vkCmdEndDebugUtilsLabelEXT' was called before being loaded"};
	#endif
	return proxy_vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}

// Proxy for device function vkCmdInsertDebugUtilsLabelEXT
static PFN_vkCmdInsertDebugUtilsLabelEXT proxy_vkCmdInsertDebugUtilsLabelEXT = nullptr;
[[maybe_unused]] static bool symbol_vkCmdInsertDebugUtilsLabelEXT = [] () {
	device_proxies["vkCmdInsertDebugUtilsLabelEXT"] = ((PFN_vkVoidFunction*) &proxy_vkCmdInsertDebugUtilsLabelEXT);
	return true;
}();
VKAPI_ATTR void VKAPI_CALL vkCmdInsertDebugUtilsLabelEXT( VkCommandBuffer commandBuffer, const VkDebugUtilsLabelEXT* pLabelInfo) {
	#if !defined(NDEBUG)
	if (!proxy_vkCmdInsertDebugUtilsLabelEXT) throw Exception {"Proxy device function 'vkCmdInsertDebugUtilsLabelEXT' was called before being loaded"};
	#endif
	return proxy_vkCmdInsertDebugUtilsLabelEXT(commandBuffer, pLabelInfo);
}



