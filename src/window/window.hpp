#pragma once

#include "external.hpp"
#include "platform.hpp"

class Window {

	public:

		READONLY GLFWwindow* glfw_window;

	public:

		Window(uint32_t w, uint32_t h, const char* title) {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

			glfw_window = glfwCreateWindow(w, h, title, nullptr, nullptr);
			if (glfw_window == nullptr) {
				throw std::runtime_error("glfwCreateWindow: Failed to create a window!");
			}

			if (!glfwVulkanSupported()) {
				throw std::runtime_error("glfwVulkanSupported: Failed to find vulkan loader!");
			}
		}

		void poll() const {
			glfwPollEvents();
		}

		bool shouldClose() const {
			return glfwWindowShouldClose(glfw_window);
		}

		void getFramebufferSize(int* width, int* height) {
			glfwGetFramebufferSize(glfw_window, width, height);
		}

		void close() const {
			glfwDestroyWindow(glfw_window);
		}


};
