#pragma once

#include "external.hpp"
#include "queues.hpp"
#include "features.hpp"
#include "sync/fence.hpp"
#include "sync/semaphore.hpp"
#include "buffer/memory.hpp"
#include "setup/picker.hpp"
#include "callback.hpp"

/**
 * A Vulkan Logical Device, a configured connection to a physical device
 */
class Device {

	public:

		READONLY VkPhysicalDevice vk_physical_device;
		READONLY VkDevice vk_device;
		READONLY ExtendedFeatureSet features;
		READONLY MemoryInfo memory;
		READONLY VkPhysicalDeviceLimits vk_limits;

	public:

		Device() = default;
		Device(VkPhysicalDevice& vk_physical_device, VkDevice& vk_device, ExtendedFeatureSet& features, VkPhysicalDeviceLimits vk_limits)
		: vk_physical_device(vk_physical_device), vk_device(vk_device), features(features), memory(vk_physical_device, vk_device), vk_limits(vk_limits) {}

		/**
		 * Get a previously requested Vulkan Queue from the device
		 */
		Queue get(QueueInfo& info, uint32_t index) {
			return {info.get(vk_device, index), info};
		}

		/**
		 * Waits (blocks) for the Device (GPU) to finish all pending work
		 */
		void wait() {
			vkDeviceWaitIdle(vk_device);
		}

		void close() {
			vkDestroyDevice(vk_device, AllocatorCallbackFactory::named("Device"));
		}

		/**
		 * A short-cut method for creating Fence objects
		 */
		Fence fence(bool signaled = false) const {
			return {vk_device, signaled};
		}

		/**
		 * A short-cut method for creating Semaphore objects
		 */
		Semaphore semaphore() const {
			return {vk_device};
		}

};

class DeviceBuilder {

	private:

		VkPhysicalDevice vk_device;
		std::vector<QueueFamily> families;

		DeviceExtensionPicker device_extensions;
		std::vector<QueueFamilyConfig> configured;

	public:

		READONLY ExtendedFeatureSet supported_features;
		READONLY ExtendedFeatureSet selected_features;
		READONLY VkPhysicalDeviceLimits vk_limits;

	public:

		/**
		 * Request a extension from the device, this is different than requesting a
		 * extension from a Instance
		 */
		Result<std::string> addDeviceExtension(const std::string& name) {
			return device_extensions.select(name);
		}

		/**
		 * Request a queue from a device and returns a reference to it,
		 * the queue will be usable only after the device is created
		 *
		 * FIXME this implementation is incorrect and buggy
		 *   * the count and priority are passed incorrectly into the vulkan structures
		 *   * this method will not work for count > 1 (possibly with a SEGFAULT)
		 */
		QueueInfo addQueue(const QueueFamilyPredicate& predicate, uint32_t count, float priority = 1.0) {
			// we can't just do it one by one, all queues for a family need to be requested at once
			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateInfo.html#VUID-VkDeviceCreateInfo-queueFamilyIndex-02802

			QueueFamilyConfig config = predicate.pick(vk_device, families)->configure(count, priority);

			// only one config per family, if two predicates matched the same queue we need to deduplicate
			if (!util::contains(configured, config)) {
				configured.push_back(config);
			}

			return config.getDelegate();
		}

		Device create() {

			std::vector<VkDeviceQueueCreateInfo> configs;

			// append configured family queues
			for (QueueFamilyConfig& family : configured) {
				configs.push_back(family.getConfig());
			}

			// information required for creating a logical device
			VkDeviceCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			create_info.pNext = selected_features.getLinked();
			create_info.pQueueCreateInfos = configs.data();
			create_info.queueCreateInfoCount = configs.size();
			create_info.pEnabledFeatures = nullptr;

			// pass extensions
			create_info.enabledExtensionCount = device_extensions.size();
			create_info.ppEnabledExtensionNames = device_extensions.data();

			// deprecated and ignored
			create_info.enabledLayerCount = 0;

			VkDevice device;
			if (vkCreateDevice(vk_device, &create_info, AllocatorCallbackFactory::named("Device"), &device) != VK_SUCCESS) {
				throw Exception {"Failed to create logical device!"};
			}

			return {vk_device, device, selected_features, vk_limits};

		}

	private:

		friend class DeviceInfo;

		DeviceBuilder(VkPhysicalDevice& vk_device, std::vector<QueueFamily>& families, ExtendedFeatureSet features, VkPhysicalDeviceLimits vk_limits)
		: vk_device(vk_device), families(families), device_extensions(vk_device), supported_features(features), vk_limits(vk_limits) {}

};

class DeviceInfo {

	private:

		VkPhysicalDevice vk_device;
		VkPhysicalDeviceProperties vk_properties;
		ExtendedFeatureSet features;
		std::vector<QueueFamily> families;

	public:

		DeviceInfo(VkPhysicalDevice& vk_device)
		: vk_device(vk_device) {

			// load info about this device into structs
			vkGetPhysicalDeviceProperties(vk_device, &vk_properties);
			vkGetPhysicalDeviceFeatures2KHR(vk_device, features.getLinked());

			// create a list of queue families supported
			uint32_t count = 0, index = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(vk_device, &count, nullptr);

			std::vector<VkQueueFamilyProperties> entires {count};
			vkGetPhysicalDeviceQueueFamilyProperties(vk_device, &count, entires.data());

			for (VkQueueFamilyProperties& family : entires) {
				families.emplace_back(family, index ++);
			}
		}

		/**
		 * @see VkPhysicalDeviceType
		 */
		VkPhysicalDeviceType getType() {
			return vk_properties.deviceType;
		}

		/**
		 * @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceProperties.html
		 */
		VkPhysicalDeviceProperties getProperties() {
			return vk_properties;
		}

		/**
		 * @see https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html
		 */
		const ExtendedFeatureSet& getFeatures() {
			return features;
		}

		/**
		 * Check if the device's swap chain is compatible with our window surface,
		 * we will consider it compatible if at least one image format and presentation mode match
		 */
		bool hasSwapchain(WindowSurface& surface) {
			uint32_t formats, modes;

			// load structure describing capabilities of a surface
			vkGetPhysicalDeviceSurfaceFormatsKHR(vk_device, surface.vk_surface, &formats, nullptr);
			vkGetPhysicalDeviceSurfacePresentModesKHR(vk_device, surface.vk_surface, &modes, nullptr);

			return formats != 0 && modes != 0;
		}

		/**
		 * Returns a queue family that supports the given queue type bit mask or the given swapchain (presentation queue),
		 * a single family can be returned for different queue types
		 */
		const QueueFamily* getQueueFamily(const QueueFamilyPredicate& predicate) {
			return predicate.pick(vk_device, families);
		}

		DeviceBuilder builder() {
			return {vk_device, families, features, vk_properties.limits};
		}

};
