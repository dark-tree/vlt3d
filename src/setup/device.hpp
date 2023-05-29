#pragma once

#include "external.hpp"
#include "queues.hpp"
#include "features.hpp"
#include "sync/fence.hpp"
#include "sync/semaphore.hpp"
#include "buffer/memory.hpp"

class Device {

	public:

		READONLY VkPhysicalDevice vk_physical_device;
		READONLY VkDevice vk_device;

	public:

		const FeatureSetView features;
		const MemoryInfo memory;

	public:

		Device(VkPhysicalDevice& vk_physical_device, VkDevice& vk_device, FeatureSetView& features)
		: vk_physical_device(vk_physical_device), vk_device(vk_device), features(features), memory(vk_physical_device, vk_device) {}

		VkQueue get(QueueInfo& info, uint32_t index) {
			return info.get(vk_device, index);
		}

		void wait() {
			vkDeviceWaitIdle(vk_device);
		}

		void close() {
			vkDestroyDevice(vk_device, nullptr);
		}

		Fence fence(bool signaled = false) {
			return {vk_device, signaled};
		}

		Semaphore semaphore() {
			return {vk_device};
		}

};

class DeviceBuilder {

	private:

		VkPhysicalDevice info;
		std::vector<QueueFamily> families;

		DeviceExtensionPicker device_extensions;
		std::vector<QueueFamilyConfig> configured;

	public:

		READONLY FeatureSet features;

	public:

		Result<std::string> addDeviceExtension(const std::string& name) {
			return device_extensions.select(name);
		}

		QueueInfo addQueue(const QueueFamilyPredicate& predicate, uint32_t count, float priority = 1.0) {
			// we can't just do it one by one, all queues for a family need to be reqested at once
			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDeviceCreateInfo.html#VUID-VkDeviceCreateInfo-queueFamilyIndex-02802

			QueueFamilyConfig config = predicate.pick(families)->configure(count, priority);

			// only one config per family, if two predicates matched the same queue we need to deduplicate
			if (!util::contains(configured, config)) {
				configured.push_back(config);
			}

			return config.getDelegate();
		}

		Device create() {

			std::vector<VkDeviceQueueCreateInfo> configs;
			FeatureSetView view = features.view();

			// append configured family queues
			for (QueueFamilyConfig& family : configured) {
				configs.push_back(family.getConfig());
			}

			// information required for creating a logical device
			VkDeviceCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			create_info.pQueueCreateInfos = configs.data();
			create_info.queueCreateInfoCount = configs.size();
			create_info.pEnabledFeatures = &view.vk_features;

			// pass extensions
			create_info.enabledExtensionCount = device_extensions.size();
			create_info.ppEnabledExtensionNames = device_extensions.data();

			// deprecated and ignored
			create_info.enabledLayerCount = 0;

			VkDevice device;
			if (vkCreateDevice(info, &create_info, nullptr, &device) != VK_SUCCESS) {
				throw std::runtime_error("vkCreateDevice: Failed to create logical device!");
			}

			return {info, device, view};

		}

	private:

		friend class DeviceInfo;

		DeviceBuilder(VkPhysicalDevice& info, std::vector<QueueFamily>& families, FeatureSet features)
		: info(info), families(families), device_extensions(info), features(features) {}

};

class DeviceInfo {

	private:

		VkPhysicalDevice info;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		std::vector<QueueFamily> families;

	public:

		DeviceInfo(VkPhysicalDevice& info)
		: info(info) {

			// load info about this device into structs
			vkGetPhysicalDeviceProperties(info, &properties);
			vkGetPhysicalDeviceFeatures(info, &features);

			// create a list of queue families supported
			uint32_t count = 0, index = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(info, &count, nullptr);

			std::vector<VkQueueFamilyProperties> entires {count};
			vkGetPhysicalDeviceQueueFamilyProperties(info, &count, entires.data());

			for (VkQueueFamilyProperties& family : entires) {
				families.emplace_back(family, index ++);
			}
		}

		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceProperties.html
		VkPhysicalDeviceProperties getProperties() {
			return properties;
		}

		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceFeatures.html
		FeatureSetView getFeatures() {
			return FeatureSet {features}.view();
		}

		bool hasSwapchain(WindowSurface& surface) {
			uint32_t formats, modes;

			// load structure describing capabilities of a surface
			vkGetPhysicalDeviceSurfaceFormatsKHR(info, surface.vk_surface, &formats, nullptr);
			vkGetPhysicalDeviceSurfacePresentModesKHR(info, surface.vk_surface, &modes, nullptr);

			return formats != 0 && modes != 0;
		}

		const QueueFamily* getQueueFamily(const QueueFamilyPredicate& predicate) {
			return predicate.pick(families);
		}

		DeviceBuilder builder() {
			return {info, families, features};
		}

};
