#pragma once

#include "external.hpp"
#include "input.hpp"
#include "util/exception.hpp"

class Window {

	public:

		READONLY GLFWwindow* glfw_window;

	private:

		READONLY InputContext input;
		InputConsumer* root;

		static void glfwKeyCallback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods);
		static void glfwButtonCallback(GLFWwindow* glfw_window, int button, int action, int mods);
		static void glfwScrollCallback(GLFWwindow* glfw_window, double x_scroll, double y_scroll);
		static void glfwErrorCallback(int code, const char* description);

	public:

		Window(uint32_t w, uint32_t h, const char* title);

		void poll() const;
		bool shouldClose() const;
		void getFramebufferSize(int* width, int* height) const;
		void close() const;
		void setTitle(const std::string& title) const;
		bool isKeyPressed(int key) const;
		bool isButtonPressed(int button) const;
		void getCursor(double* x, double* y) const;
		void setRootInputConsumer(NULLABLE InputConsumer* root);
		void setMouseCapture(bool capture);

		InputContext& getInputContext();

};
