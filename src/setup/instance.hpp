#pragma once

#include "external.hpp"
#include "util/util.hpp"
#include "picker.hpp"
#include "proxy.hpp"
#include "messanger.hpp"
#include "device.hpp"
#include "surface.hpp"
#include "window/window.hpp"

/**
 * A structure describing the application
 * and the connection with the Vulkan API
 */
class Instance {

	public:

		READONLY VkInstance vk_instance;

	private:

		DebugMessenger messenger;
		bool validation;
		std::vector<DeviceInfo> devices;

	public:

		Instance() = default;
		Instance(VkInstance vk_instance, DebugMessenger messenger, bool validation)
		: vk_instance(vk_instance), messenger(messenger), validation(validation) {

			uint32_t count = 0;
			vkEnumeratePhysicalDevices(vk_instance, &count, nullptr);

			std::vector<VkPhysicalDevice> entries {count};
			vkEnumeratePhysicalDevices(vk_instance, &count, entries.data());

			devices.reserve(count);
			for (VkPhysicalDevice& device : entries) {
				devices.emplace_back(device);
			}

		}

		void close() {
			messenger.close();
			vkDestroyInstance(vk_instance, AllocatorCallbackFactory::named("Instance"));
		}

		/**
		 * Get a list of Physical Devices that support our Instance configuration
		 */
		std::vector<DeviceInfo>& getDevices() {
			return devices;
		}

		/**
		 * Get a representation of a connection between Vulkan API and our window
		 */
		WindowSurface createSurface(Window& window) {
			return {window, vk_instance};
		}

};

class InstanceBuilder {

	private:

		uint32_t version = VK_API_VERSION_1_0;
		const char* name = "Unknown";
		uint16_t major = 0, minor = 0, patch = 0;

		InstanceExtensionPicker instance_extensions;
		ValidationLayerPicker validation_layers;
		DebugMessengerConfig messenger;

	private:

		std::vector<const char*> getRequiredExtensions() {
			uint32_t count = 0;
			const char** array = glfwGetRequiredInstanceExtensions(&count);
			return {array, array + count};
		}

	public:

		/**
		 * Set the required Vulkan API version
		 * Use the `VK_API_VERSION_*` defines, defaults to VK_API_VERSION_1_0
		 */
		void setVulkanVersion(uint32_t version) {
			this->version = version;
		}

		/**
		 * Provide a basic and optional description of the application.
		 * This information CAN be used by the graphics driver
		 */
		void addApplicationInfo(const char* name, uint16_t major = 1, uint16_t minor = 0, uint16_t patch = 0) {
			this->name = name;
			this->major = major;
			this->minor = minor;
			this->patch = patch;
		}

		/**
		 * Set a error handler for when a Vulkan API is incorrectly used,
		 * some validation layer should be enabled for this call to be useful
		 */
		void addDebugMessenger(const DebugMessengerConfig::CallbackFunction& callback = DebugMessengerConfig::DefaultExt) {
			messenger.configure(callback);
		}

		/**
		 * Add instance-level extensions, this is internally used for adding extensions needed
		 * by the window creation library and for the 'VK_KHR_*_surface' and 'VK_EXT_debug_utils' extensions
		 */
		Result<std::string> addInstanceExtension(const std::string& name) {
			return instance_extensions.select(name);
		}

		/**
		 * Enable error checking validation layer, without validation layers Vulkan API will silently fail
		 * and any non-valid usage will produce undefined behaviour
		 */
		Result<std::string> addValidationLayer(const std::string& name) {
			return validation_layers.select(name);
		}

		Instance build() {

			// validation requires this extension to be enabled
			if (validation_layers.isAnySelected()) {
				addInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME).orFail();
			}

			#ifdef API_WIN32
				addInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME).orFail();
			#endif

			#ifdef API_XLIB
				addInstanceExtension(VK_KHR_XLIB_SURFACE_EXTENSION_NAME).orWarn();
			#endif

			#ifdef API_WAYLAND
				addInstanceExtension(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME).orWarn();
			#endif

			// make sure extensions required by GLFW are available
			for (const char* name : getRequiredExtensions()) {
				addInstanceExtension(name).orFail();
			}

			// optional, general info about the application
			VkApplicationInfo app_info {};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = name;
			app_info.applicationVersion = VK_MAKE_VERSION(major, minor, patch);
			app_info.pEngineName = "VLT3D";
			app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			app_info.apiVersion = this->version;

			// information required for creating an instance
			VkInstanceCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			create_info.pApplicationInfo = &app_info;
			create_info.enabledExtensionCount = instance_extensions.size();
			create_info.ppEnabledExtensionNames = instance_extensions.data();
			create_info.enabledLayerCount = validation_layers.size();
			create_info.ppEnabledLayerNames = validation_layers.data();

			// attach debug messenger if configured
			if (messenger.isEnabled()) {
				create_info.pNext = messenger.getConfigPointer();
			}

			VkInstance instance;
			if (vkCreateInstance(&create_info, AllocatorCallbackFactory::named("Instance"), &instance) != VK_SUCCESS) {
				throw Exception {"Failed to create Vulkan instance"};
			}

			// we need to fetch the functions and this is the earliest point we have the VkInstance
			loadInstanceFunction(instance, "vkCreateDebugUtilsMessengerEXT");
			loadInstanceFunction(instance, "vkDestroyDebugUtilsMessengerEXT");
			loadInstanceFunction(instance, "vkGetPhysicalDeviceFeatures2KHR");

			return Instance {instance, messenger.attach(instance), validation_layers.isAnySelected()};

		}

};
