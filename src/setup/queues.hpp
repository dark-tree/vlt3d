#pragma once

#include "surface.hpp"

class QueueInfo {

	public:

		READONLY const uint32_t index;
		READONLY const uint32_t count;

	public:

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

		// TODO implement
		bool canPresentTo(WindowSurface& surface) const {
			VkBool32 supported = true;
			//vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &supported);
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

		const std::function<bool (QueueFamily&)> tester;

	public:

		QueueFamilyPredicate(QueueType flags)
		: tester([flags] (QueueFamily& family) { return (family.getFlags() & (VkQueryControlFlagBits) flags); }) {}

		QueueFamilyPredicate(WindowSurface& surface)
		: tester([&] (QueueFamily& family) { return family.canPresentTo(surface); }) {}

		bool test(QueueFamily& family) const {
			return tester(family);
		}

	public:

		/**
		 * Picks a matching QueueFamily from a vector and returns it, otherwise returns a nullptr
		 */
		QueueFamily* pick(std::vector<QueueFamily>& families) const {

			for (QueueFamily& family : families) {
				if (test(family)) return &family;
			}

			return nullptr;
		}

};
