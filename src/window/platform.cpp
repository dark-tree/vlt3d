
#include "system.hpp"

#ifdef API_WIN32
#	define GLFW_EXPOSE_NATIVE_WIN32
#	define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifdef API_XLIB
#	define GLFW_EXPOSE_NATIVE_X11
#	define VK_USE_PLATFORM_XLIB_KHR
#endif

#ifdef API_WAYLAND
#	define GLFW_EXPOSE_NATIVE_WAYLAND
#	define VK_USE_PLATFORM_WAYLAND_KHR
#endif

#include "external.hpp"
#include "platform.hpp"

// a hack to make this work while i think of a better way to fix this
// https://stackoverflow.com/a/43854514

#ifdef API_XLIB
	Display* glfwGetX11Display() __attribute__((weak));
	Window glfwGetX11Window(GLFWwindow* window) __attribute__((weak));
#endif

#ifdef API_WAYLAND
	wl_display* glfwGetWaylandDisplay() __attribute__((weak));
	wl_surface* glfwGetWaylandWindow(GLFWwindow* window) __attribute__((weak));
#endif

#ifdef API_WIN32
	void platform::createSurfaceWin32(GLFWwindow* window, VkInstance instance, VkSurfaceKHR* surface) {

		VkWin32SurfaceCreateInfoKHR create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		create_info.hwnd = glfwGetWin32Window(window);
		create_info.hinstance = GetModuleHandle(nullptr);

		if (vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("vkCreateWin32SurfaceKHR: Failed to create Win32 window surface!");
		}

	}
#endif

#ifdef API_XLIB
	void platform::createSurfaceXlib(GLFWwindow* window, VkInstance instance, VkSurfaceKHR* surface) {

		VkXlibSurfaceCreateInfoKHR create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		create_info.dpy = glfwGetX11Display();
		create_info.window = glfwGetX11Window(window);

		if (vkCreateXlibSurfaceKHR(instance, &create_info, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("vkCreateXlibSurfaceKHR: Failed to create Xlib window surface!");
		}

	}
#endif

#ifdef API_WAYLAND
	void platform::createSurfaceWayland(GLFWwindow* window, VkInstance instance, VkSurfaceKHR* surface) {

		VkWaylandSurfaceCreateInfoKHR create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
		create_info.display = glfwGetWaylandDisplay();
		create_info.surface = glfwGetWaylandWindow(window);

		if (vkCreateWaylandSurfaceKHR(instance, &create_info, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("vkCreateWaylandSurfaceKHR: Failed to create Wayland window surface!");
		}

	}
#endif
