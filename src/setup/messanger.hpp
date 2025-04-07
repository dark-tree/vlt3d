#pragma once

#include "external.hpp"
#include "util/exception.hpp"
#include "setup/callback.hpp"

/**
 * Warning: This class uses a destructor!
 */
class DebugMessenger {

	private:

		static constexpr const char* PREFIX = "[Vulkan]";

		VkInstance vk_instance;
		VkDebugUtilsMessengerEXT vk_messenger;

		std::atomic_int error_count = 0;
		std::atomic_int warning_count = 0;

	private:

		friend class DebugMessengerConfig;

		void set(VkInstance vk_instance, VkDebugUtilsMessengerEXT vk_messenger);
		VkBool32 onMessage(VkDebugUtilsMessageSeverityFlagBitsEXT severity, const VkDebugUtilsMessengerCallbackDataEXT* data);

	public:

		DebugMessenger() = default;

		/**
		 * this classes use a destructor, close it with 'delete messenger'
		 */
		~DebugMessenger();

		/**
		 * Returns the number of errors that have occurred up to this point
		 */
		int getErrorCount() const;

};

class DebugMessengerConfig {

	private:

		VkDebugUtilsMessengerCreateInfoEXT create_info {};
		DebugMessenger* messenger = nullptr;

		/**
		 * This function matched the signature expected by vulkan messenger extension
		 * and is used as a callback that then dispatches the message to the DebugMessenger#onMessage()
		 */
		static VKAPI_ATTR VkBool32 VKAPI_CALL MessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void* user);

	public:

		/**
		 * Enables the debug messenger
		 * add getConfigPointer() to the instance pNext chain and call attach() after it is created
		 */
		void enable();

	private:

		friend class InstanceBuilder;

		/// Creates the messenger objects
		DebugMessenger* attach(VkInstance vk_instance);

		/// Check if the messenger has been enabled
		bool isEnabled() const;

		/// Returns the pointer to a structure that needs to be included in instance pNext chain
		VkDebugUtilsMessengerCreateInfoEXT* getConfigPointer();

};
