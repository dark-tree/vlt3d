
namespace platform {

// extracted from vulkan headers, bc C/C++ is shit
// https://github.com/vulkan-go/vulkan/tree/master/vulkan
#define VK_KHR_XLIB_SURFACE_EXTENSION_NAME "VK_KHR_xlib_surface"
#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"
#define VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME "VK_KHR_wayland_surface"

#ifdef API_WIN32
	void createSurfaceWin32(GLFWwindow* window, VkInstance instance, VkSurfaceKHR* surface);
#endif

#ifdef API_X11
	void createSurfaceXlib(GLFWwindow* window, VkInstance instance, VkSurfaceKHR* surface);
#endif

#ifdef API_WAYLAND
	void createSurfaceWayland(GLFWwindow* window, VkInstance instance, VkSurfaceKHR* surface);
#endif

}
