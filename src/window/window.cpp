
#include "window.hpp"

void Window::glfwKeyCallback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods) {
	Window* window = (Window*) glfwGetWindowUserPointer(glfw_window);

	if (window && window->root) {
		window->root->onEvent(window->input, KeyboardEvent {key, mods, action});
	}
}

void Window::glfwButtonCallback(GLFWwindow* glfw_window, int button, int action, int mods) {
	Window* window = (Window*) glfwGetWindowUserPointer(glfw_window);

	if (window && window->root) {
		window->root->onEvent(window->input, MouseEvent {button, mods, action});
	}
}

void Window::glfwScrollCallback(GLFWwindow* glfw_window, double x_scroll, double y_scroll) {
	Window* window = (Window*) glfwGetWindowUserPointer(glfw_window);

	if (window && window->root) {
		window->root->onEvent(window->input, ScrollEvent {(float) y_scroll});
	}
}

Window::Window(uint32_t w, uint32_t h, const char* title)
: input(*this), root(nullptr) {
	// one-of init
	static auto init = [] {
		return glfwInit();
	} ();

	if (!init) {
		throw Exception {"Failed to initialize!"};
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfw_window = glfwCreateWindow(w, h, title, nullptr, nullptr);
	if (glfw_window == nullptr) {
		throw Exception {"Failed to create a window!"};
	}

	glfwSetWindowUserPointer(glfw_window, this);
//	glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, GL_TRUE);

	if (!glfwVulkanSupported()) {
		throw Exception {"Failed to find vulkan loader!"};
	}

	// send events to the root input consumer
	glfwSetKeyCallback(glfw_window, glfwKeyCallback);
	glfwSetMouseButtonCallback(glfw_window, glfwButtonCallback);
	glfwSetScrollCallback(glfw_window, glfwScrollCallback);

	//glfwSwapInterval(0);
}

bool Window::isPressed(int key) const {
	return isKeyPressed(key);
}

void Window::poll() const {
	glfwPollEvents();
}

bool Window::shouldClose() const {
	return glfwWindowShouldClose(glfw_window);
}

void Window::getFramebufferSize(int* width, int* height) const {
	glfwGetFramebufferSize(glfw_window, width, height);
}

void Window::close() const {
	glfwDestroyWindow(glfw_window);
}

void Window::setTitle(const std::string& title) const {
	glfwSetWindowTitle(glfw_window, title.c_str());
}

bool Window::isKeyPressed(int key) const {
	return glfwGetKey(glfw_window, key) == GLFW_PRESS;
}

bool Window::isButtonPressed(int button) const {
	return glfwGetMouseButton(glfw_window, button) == GLFW_PRESS;
}

void Window::getCursor(double* x, double* y) const {
	glfwGetCursorPos(glfw_window, x, y);
}

void Window::setRootInputConsumer(NULLABLE InputConsumer* root) {
	this->root = root;
}

InputContext& Window::getInputContext() {
	return input;
}