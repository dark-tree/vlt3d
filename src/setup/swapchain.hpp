#pragma once

#define SWAPCHAIN_EXTENT_AUTO 0xFFFFFFFF

#include "device.hpp"
#include "queues.hpp"
#include "surface.hpp"
#include "render/image.hpp"
#include "sync/semaphore.hpp"

class Swapchain {

	public:

		READONLY VkSwapchainKHR vk_swapchain;
		READONLY VkSurfaceFormatKHR vk_surface_format;
		READONLY VkExtent2D vk_extent;
		READONLY VkDevice vk_device;

	private:

		std::vector<Image> images;

	public:

		Swapchain(VkSwapchainKHR vk_swapchain, VkSurfaceFormatKHR vk_surface_format, VkExtent2D vk_extent, VkDevice vk_device)
		: vk_swapchain(vk_swapchain), vk_surface_format(vk_surface_format), vk_extent(vk_extent), vk_device(vk_device) {

			uint32_t count;

			vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &count, nullptr);

			if (count > 0) {
				std::vector<VkImage> entries {count};
				vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &count, entries.data());

				images.reserve(count);

				for (VkImage img : entries) {
					images.emplace_back(img, vk_surface_format.format);
				}
			}

		}

		std::vector<Image>& getImages() {
			return images;
		}

		uint32_t getNextImage(Semaphore& semaphore) {
			uint32_t index;
   			vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, semaphore.vk_semaphore, VK_NULL_HANDLE, &index);

			return index;
		}

		VkResult present(VkQueue queue, Semaphore& await, uint32_t image_index) {

			VkPresentInfoKHR info {};
			info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			info.waitSemaphoreCount = 1;
			info.pWaitSemaphores = &await.vk_semaphore;

			info.swapchainCount = 1;
			info.pSwapchains = &vk_swapchain;
			info.pImageIndices = &image_index;
			info.pResults = nullptr;

			return vkQueuePresentKHR(queue, &info);

		}

		void close() {
			vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr);
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

		void addSyncedQueue(QueueInfo& queue) {

			for (uint32_t family : families) {
				if (queue.index == family) return;
			}

			families.push_back(queue.index);

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

			// ignore pixels that are obsured (for example by a window on top)
			create_info.clipped = VK_TRUE;

			VkSwapchainKHR swapchain;

			if (vkCreateSwapchainKHR(device.vk_device, &create_info, nullptr, &swapchain) != VK_SUCCESS) {
				throw std::runtime_error("vkCreateSwapchainKHR: Failed to create swapchain!");
			}

			return Swapchain {swapchain, format, extent, device.vk_device};

		}

};

class SwapchainInfo {

	private:

		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> modes;

	public:

		SwapchainInfo(const VkPhysicalDevice& device, WindowSurface& surface) {

			uint32_t count;

			// load structure describing capabilities of a surface
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface.vk_surface, &capabilities);

			// load structures describing a supported swapchain format-color space pair
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.vk_surface, &count, nullptr);

			if (count != 0) {
				formats.resize(count);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.vk_surface, &count, formats.data());
			}

			// load strucutres describing supported presentation modes of a surface
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.vk_surface, &count, nullptr);

			if (count != 0) {
				modes.resize(count);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.vk_surface, &count, modes.data());
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
