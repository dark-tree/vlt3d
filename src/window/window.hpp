#pragma once

#include "external.hpp"
#include "platform.hpp"

class Window {

	public:

		READONLY GLFWwindow* glfw_window;

	public:

		Window(uint32_t w, uint32_t h, const char* title) {

			// one-of init
			static auto init = [] {
				stbi_flip_vertically_on_write(true);
				stbi_set_flip_vertically_on_load(true);

				return glfwInit();
			} ();

			if (!init) {
				throw std::runtime_error {"glfwInit: Failed to initialize!"};
			}

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

			glfw_window = glfwCreateWindow(w, h, title, nullptr, nullptr);
			if (glfw_window == nullptr) {
				throw std::runtime_error("glfwCreateWindow: Failed to create a window!");
			}

			glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, GL_TRUE);

			if (!glfwVulkanSupported()) {
				throw std::runtime_error("glfwVulkanSupported: Failed to find vulkan loader!");
			}

			//glfwSwapInterval(0);
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

		void setTitle(const std::string& title) const {
			glfwSetWindowTitle(glfw_window, title.c_str());
		}

		bool isPressed(int key) const {
			return glfwGetKey(glfw_window, key) == GLFW_PRESS;
		}

		void getCursor(double* x, double* y) const {
			glfwGetCursorPos(glfw_window, x, y);
		}

};
