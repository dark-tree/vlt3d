#pragma once

#include "external.hpp"
#include "window/window.hpp"
#include "window/platform.hpp"
#include "util/logger.hpp"
#include "callback.hpp"

class WindowSurface {

	private:

		using SurfaceConstructor = void (*) (GLFWwindow*, VkInstance, VkSurfaceKHR*);

		using Entry = std::pair<int, SurfaceConstructor>;

		constexpr static std::array constructors = {
			#ifdef API_WIN32
				Entry {GLFW_PLATFORM_WIN32, platform::createSurfaceWin32},
			#endif

			#ifdef API_X11
				Entry {GLFW_PLATFORM_X11, platform::createSurfaceXlib},
			#endif

			#ifdef API_WAYLAND
				Entry {GLFW_PLATFORM_WAYLAND, platform::createSurfaceWayland},
			#endif
		};

	public:

		READONLY VkSurfaceKHR vk_surface;
		READONLY VkInstance vk_instance;

	public:

		WindowSurface() = default;
		WindowSurface(Window& window, VkInstance& vk_instance)
		: vk_instance(vk_instance) {
			bool created = false;
			int platform = glfwGetPlatform();

			for (const Entry& entry : constructors) {
				if (platform == entry.first) {
					entry.second(window.glfw_window, vk_instance, &vk_surface);
					created = true;
					break;
				}
			}

			if (!created) {
				logger::fatal("Unable to create window surface with the platform used by GLFW (id: ", platform, ")");
				throw Exception {"No supported Window API found!"};
			}
		}

		void close() {
			vkDestroySurfaceKHR(vk_instance, vk_surface, AllocatorCallbackFactory::named("SurfaceKHR"));
		}
};
