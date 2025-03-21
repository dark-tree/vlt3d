#pragma once

#include "surface.hpp"

class QueueInfo {

	public:

		READONLY uint32_t index;
		READONLY uint32_t count;

	public:

		QueueInfo() = default;
		QueueInfo(uint32_t index, uint32_t count)
		: index(index), count(count) {}

		VkQueue get(VkDevice& device, uint32_t id) const {

			if (id >= count) {
				throw std::runtime_error("vkGetDeviceQueue: Invalid queue index!");
			}

			// query the queue from the device
			VkQueue queue;
			vkGetDeviceQueue(device, index, id, &queue);

			return queue;

		}

};

class QueueFamilyConfig {

	private:

		VkDeviceQueueCreateInfo create_info;
		const float priority;

	public:

		QueueFamilyConfig(VkDeviceQueueCreateInfo create_info, float priority)
		: create_info(create_info), priority(priority) {}

		// we set the priority here to make sure realocations don't break anything
		VkDeviceQueueCreateInfo& getConfig() {
			create_info.pQueuePriorities = &priority;
			return create_info;
		}

		QueueInfo getDelegate() {
			return {create_info.queueFamilyIndex, create_info.queueCount};
		}

	public:

		// needed for DeviceBuilder::addQueue, don't actually use this
		bool operator == (const QueueFamilyConfig& rhs) const {
			return create_info.queueFamilyIndex == rhs.create_info.queueFamilyIndex;
		}

};

class QueueFamily {

	private:

		const VkQueueFamilyProperties properties;
		const uint32_t index;

	public:

		QueueFamily(VkQueueFamilyProperties& properties, uint32_t index)
		: properties(properties), index(index) {}

		VkQueueFlags getFlags() const {
			return properties.queueFlags;
		}

		size_t getMaxCount() const {
			return properties.queueCount;
		}

		bool canPresentTo(VkPhysicalDevice device, WindowSurface& surface) const {
			VkBool32 supported = true;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface.vk_surface, &supported);
			return supported;
		}

		QueueFamilyConfig configure(uint32_t count, float priority) const {

			VkDeviceQueueCreateInfo create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			create_info.queueFamilyIndex = index;
			create_info.queueCount = count;

			return {create_info, priority};

		}

};

enum struct QueueType {
	GRAPHICS = VK_QUEUE_GRAPHICS_BIT,
	COMPUTE = VK_QUEUE_COMPUTE_BIT,
	TRANSFER = VK_QUEUE_TRANSFER_BIT,
	SPARSE_BINDING = VK_QUEUE_SPARSE_BINDING_BIT,
	PROTECTED = VK_QUEUE_PROTECTED_BIT
};

class QueueFamilyPredicate {

	private:

		const std::function<bool (VkPhysicalDevice device, QueueFamily&)> tester;

	public:

		QueueFamilyPredicate(QueueType flags)
		: tester([flags] (auto device, auto& family) { return (family.getFlags() & (VkQueryControlFlagBits) flags); }) {}

		QueueFamilyPredicate(WindowSurface& surface)
		: tester([&] (auto device, auto& family) { return family.canPresentTo(device, surface); }) {}

		bool test(VkPhysicalDevice device, QueueFamily& family) const {
			return tester(device, family);
		}

	public:

		/**
		 * Picks a matching QueueFamily from a vector and returns it, otherwise returns a nullptr
		 */
		QueueFamily* pick(VkPhysicalDevice device, std::vector<QueueFamily>& families) const {

			for (QueueFamily& family : families) {
				if (test(device, family)) return &family;
			}

			return nullptr;
		}

};

class Queue {

	public:

		READONLY VkQueue vk_queue;
		READONLY QueueInfo info;

	public:

		Queue() = default;
		Queue(VkQueue vk_queue, QueueInfo& info)
		: vk_queue(vk_queue), info(info) {}

};
