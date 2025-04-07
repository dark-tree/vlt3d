#pragma once

#define SWAPCHAIN_EXTENT_AUTO 0xFFFFFFFF

#include "device.hpp"
#include "queues.hpp"
#include "present.hpp"
#include "surface.hpp"
#include "buffer/image.hpp"
#include "sync/semaphore.hpp"
#include "render/framebuffer.hpp"

class Swapchain {

	public:

		READONLY VkSwapchainKHR vk_swapchain;
		READONLY VkSurfaceFormatKHR vk_surface_format;
		READONLY VkExtent2D vk_extent;

	private:

		std::vector<Image> images;
		std::vector<ImageView> views;
		Device* device;

	public:

		Swapchain() = default;
		Swapchain(VkSwapchainKHR vk_swapchain, VkSurfaceFormatKHR vk_surface_format, VkExtent2D vk_extent, Device& device)
		: vk_swapchain(vk_swapchain), vk_surface_format(vk_surface_format), vk_extent(vk_extent), device(&device) {

			uint32_t count;

			vkGetSwapchainImagesKHR(device.vk_device, vk_swapchain, &count, nullptr);

			if (count > 0) {
				std::vector<VkImage> entries {count};
				vkGetSwapchainImagesKHR(device.vk_device, vk_swapchain, &count, entries.data());

				images.reserve(count);

				for (VkImage vk_image : entries) {
					Image image {vk_image, vk_surface_format.format};
					ImageView view = image.getViewBuilder().build(device, VK_IMAGE_ASPECT_COLOR_BIT);

					images.push_back(image);
					views.push_back(view);

					#if !defined(NDEBUG)
					std::string name {"Swapchain #"};
					name += std::to_string(images.size() - 1);
					image.setDebugName(device, name.c_str());
					view.setDebugName(device, name.c_str());
					#endif
				}
			}

		}

		const std::vector<ImageView>& getImageViews() {
			return views;
		}

		PresentResult getNextImage(Semaphore& semaphore, uint32_t* image_index) {
			return {vkAcquireNextImageKHR(device->vk_device, vk_swapchain, UINT64_MAX, semaphore.vk_semaphore, VK_NULL_HANDLE, image_index)};
		}

		PresentResult present(Queue queue, const Semaphore& await, uint32_t image_index) {

			VkPresentInfoKHR info {};
			info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			info.waitSemaphoreCount = 1;
			info.pWaitSemaphores = &await.vk_semaphore;

			info.swapchainCount = 1;
			info.pSwapchains = &vk_swapchain;
			info.pImageIndices = &image_index;
			info.pResults = nullptr;

			return {vkQueuePresentKHR(queue.vk_queue, &info)};

		}

		void close() {
			for (ImageView& view : views) {
				view.close(*device);
			}

			vkDestroySwapchainKHR(device->vk_device, vk_swapchain, AllocatorCallbackFactory::named("Swapchain"));
		}

};

class SwapchainBuilder {

	private:

		const VkPresentModeKHR mode;
		const VkSurfaceFormatKHR format;
		const VkExtent2D extent;
		const uint32_t images;
		const VkImageUsageFlags usage;
		const VkSurfaceTransformFlagBitsKHR transform;

		std::vector<uint32_t> families;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;

	public:

		SwapchainBuilder(VkPresentModeKHR mode, VkSurfaceFormatKHR format, VkExtent2D extent, uint32_t images, VkImageUsageFlags usage, VkSurfaceTransformFlagBitsKHR transform)
		: mode(mode), format(format), extent(extent), images(images), usage(usage), transform(transform) {}

		void addSyncedQueue(Queue queue) {
			QueueInfo info = queue.info;

			for (uint32_t family : families) {
				if (info.index == family) return;
			}

			families.push_back(info.index);

		}

		void setParent(VkSwapchainKHR swapchain) {
			this->swapchain = swapchain;
		}

		Swapchain build(Device& device, WindowSurface& surface) {

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSwapchainCreateInfoKHR.html
			VkSwapchainCreateInfoKHR create_info {};
			create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			create_info.surface = surface.vk_surface;

			// basic creation information
			create_info.minImageCount = images;
			create_info.imageFormat = format.format;
			create_info.imageColorSpace = format.colorSpace;
			create_info.imageExtent = extent;
			create_info.imageArrayLayers = 1;
			create_info.imageUsage = usage;
			create_info.presentMode = mode;
			create_info.preTransform = transform;

			// no synchonization needed, we operate on a single queue family
			if (families.size() <= 1) {
				create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				create_info.queueFamilyIndexCount = 0; // optional
				create_info.pQueueFamilyIndices = nullptr; // optional
			} else {
				create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				create_info.queueFamilyIndexCount = families.size();
				create_info.pQueueFamilyIndices = families.data();
			}

			// how to handle pixels behind the window
			create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

			// ignore pixels that are obscured (for example by a window on top)
			create_info.clipped = VK_TRUE;

			VkSwapchainKHR swapchain;

			if (vkCreateSwapchainKHR(device.vk_device, &create_info, AllocatorCallbackFactory::named("Swapchain"), &swapchain) != VK_SUCCESS) {
				throw Exception {"Failed to create swapchain!"};
			}

			return Swapchain {swapchain, format, extent, device};

		}

};

class SwapchainInfo {

	private:

		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> modes;

	public:

		SwapchainInfo(const Device& device, WindowSurface& surface) {

			uint32_t count;
			VkPhysicalDevice vk_device = device.vk_physical_device;

			// load structure describing capabilities of a surface
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_device, surface.vk_surface, &capabilities);

			// load structures describing a supported swapchain format-color space pair
			vkGetPhysicalDeviceSurfaceFormatsKHR(vk_device, surface.vk_surface, &count, nullptr);

			if (count != 0) {
				formats.resize(count);
				vkGetPhysicalDeviceSurfaceFormatsKHR(vk_device, surface.vk_surface, &count, formats.data());
			}

			// load strucutres describing supported presentation modes of a surface
			vkGetPhysicalDeviceSurfacePresentModesKHR(vk_device, surface.vk_surface, &count, nullptr);

			if (count != 0) {
				modes.resize(count);
				vkGetPhysicalDeviceSurfacePresentModesKHR(vk_device, surface.vk_surface, &count, modes.data());
			}

		}

		bool isValid() const {
			return !formats.empty() && !modes.empty();
		}

		std::vector<VkSurfaceFormatKHR>& getFormats() {
			return formats;
		}

		std::vector<VkPresentModeKHR>& getModes() {
			return modes;
		}

		VkExtent2D getExtent(Window& window) const {

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSurfaceCapabilitiesKHR.html
			// check agains the special value specified in the spec
			if (capabilities.currentExtent.width == SWAPCHAIN_EXTENT_AUTO && capabilities.currentExtent.height == SWAPCHAIN_EXTENT_AUTO) {
				return capabilities.currentExtent;
			}

			int width, height;
			window.getFramebufferSize(&width, &height);

			VkExtent2D extent {};
			extent.width = std::clamp((uint32_t) width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height = std::clamp((uint32_t) height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return extent;

		}

		uint32_t getImageCount(uint32_t extra = 1) const {

			// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSurfaceCapabilitiesKHR.html
			// is maxImageCount is 0 that means that there is no limit (other than free memory)
			const uint32_t min_images = capabilities.minImageCount;
			const uint32_t max_images = capabilities.maxImageCount == 0 ? 0xffff : capabilities.maxImageCount;

			return std::clamp(min_images + extra, min_images, max_images);

		}

		VkSurfaceTransformFlagBitsKHR getTransform() {
			return capabilities.currentTransform;
		}

};
