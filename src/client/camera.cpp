
#include "camera.hpp"
#include "util/logger.hpp"

void Camera::update() {
	
	float time_delta = getTimeDelta();
	glm::vec2 mouse_delta = getMouseDelta();

	// calculate and clamp the camera angle, in degrees
	angle.x = angle.x - mouse_delta.x;
	angle.y = std::clamp(angle.y - mouse_delta.y, -89.0f, +89.0f);

	// calculate rotation, in radians
	float rx = glm::radians(angle.x);
	float ry = glm::radians(angle.y);
	this->rotation.x = rx + glm::radians(-90.0f);
	this->rotation.y = ry;

	// vector representing where the camera is currently pointing
	this->direction = glm::normalize(glm::vec3 {
		 cos(rx) * cos(ry),
		-sin(ry),
		 sin(rx) * cos(ry)
	});

	const float multiplier = speed * time_delta;

	// keyboard input
	if (window.isKeyPressed(GLFW_KEY_W)) {
		this->pos += this->direction * multiplier;
	}

	if (window.isKeyPressed(GLFW_KEY_S)) {
		this->pos -= this->direction * multiplier;
	}

	if (window.isKeyPressed(GLFW_KEY_D)) {
		glm::vec3 vec = glm::cross(this->direction, glm::vec3(0, 1, 0));
		this->pos -= glm::normalize(vec) * multiplier;
	}

	if (window.isKeyPressed(GLFW_KEY_A)) {
		glm::vec3 vec = glm::cross(this->direction, glm::vec3(0, 1, 0));
		this->pos += glm::normalize(vec) * multiplier;
	}

	// save current view
	if (window.isKeyPressed(GLFW_KEY_B)) {
		view = getView();
	}

	// enter saved view
	if (window.isKeyPressed(GLFW_KEY_N) && !locked) {
		logger::info("[Camera] Entering saved view");
		locked = true;
	}

	// exit saved view
	if (window.isKeyPressed(GLFW_KEY_M) && locked) {
		logger::info("[Camera] Exiting saved view");
		locked = false;
	}

	this->pos.y -= window.isKeyPressed(GLFW_KEY_Q) ? multiplier : 0;
	this->pos.y += window.isKeyPressed(GLFW_KEY_E) ? multiplier : 0;
}

float Camera::getTimeDelta() {
	const double now = glfwGetTime();

	float delta = now - this->last_time;
	this->last_time = now;
	return delta;
}

glm::vec2 Camera::getMouseDelta() {
	glm::vec2 mouse = window.getInputContext().getMouse();
	glm::vec2 delta = (cursor - mouse) * glm::vec2 {-1, +1} * sensitivity;
	cursor = mouse;

	return delta;
}

Camera::Camera(Window& window, float sensitivity, float speed)
: window(window) {

	this->angle = glm::vec2(0);
	this->rotation = glm::vec3(0);
	this->pos = glm::vec3(2, 2, -2);

	this->sensitivity = sensitivity;
	this->speed = speed;

	this->cursor = window.getInputContext().getMouse();
	this->last_time = glfwGetTime();
}

void Camera::move(const glm::vec3& pos) {
	this->pos = pos;
}

glm::vec3 Camera::getPosition() const {
	return this->pos;
}

glm::vec3 Camera::getDirection() const {
	return this->direction;
}

glm::vec3 Camera::getUp() const {
	glm::vec3 sideways = glm::cross(this->direction, glm::vec3(0, 1, 0));
	return glm::cross(this->direction, sideways);
}

glm::mat4 Camera::getView() const {
	if (locked) {
		return view;
	}

	return glm::lookAt(this->pos, this->pos + this->direction, glm::vec3(0.0f, -1.0f, 0.0f));
}

Frustum Camera::getFrustum(glm::mat4& projection) const {
	return {projection * glm::lookAt(this->pos, this->pos + this->direction, glm::vec3(0.0f, -1.0f, 0.0f))};
}



