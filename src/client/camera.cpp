
#include "camera.hpp"

void Camera::update() {
	
	this->updateTime();
	float x = 0, y = 0;

	// get cursors position
	getCursorPos(&x, &y);
	double delta_x = -1.0f * (cursor.x - x) * this->sensivity;
	double delta_y = +1.0f * (cursor.y - y) * this->sensivity;
	cursor.x = x;
	cursor.y = y;

	x = angle.x - delta_x;
	y = angle.y - delta_y;

	// limit viewing angles
	if (y > +89.0) y = +89.0;
	if (y < -89.0) y = -89.0;

	this->angle.x = x;
	this->angle.y = y;

	// calculate rotation
	x = glm::radians(x);
	y = glm::radians(y);
	this->rotation.x = x + glm::radians(-90.0f);
	this->rotation.y = y;

	// vector representing where the camera is currently pointing
	this->direction = {
		 cos(x) * cos(y),
		-sin(y),
		 sin(x) * cos(y)
	};

	this->direction = glm::normalize(this->direction);

	const float speed = this->speed * this->delta_time;

	//keyboard input
	if (window.isKeyPressed(GLFW_KEY_W)) {
		this->pos += this->direction * speed;
	}

	if (window.isKeyPressed(GLFW_KEY_S)) {
		this->pos -= this->direction * speed;
	}

	if (window.isKeyPressed(GLFW_KEY_D)) {
		glm::vec3 vec = glm::cross(this->direction, glm::vec3(0, 1, 0));
		this->pos -= glm::normalize(vec) * speed;
	}

	if (window.isKeyPressed(GLFW_KEY_A)) {
		glm::vec3 vec = glm::cross(this->direction, glm::vec3(0, 1, 0));
		this->pos += glm::normalize(vec) * speed;
	}

	this->pos.y -= (window.isKeyPressed(GLFW_KEY_Q)) ? speed : 0;
	this->pos.y += (window.isKeyPressed(GLFW_KEY_E)) ? speed : 0;
}

void Camera::getCursorPos(float* x, float* y) {
	double cx, cy;
	window.getCursor(&cx, &cy);

	*x = (float) cx;
	*y = (float) cy;
}

Camera::Camera(Window& window, float sensitivity, float speed)
: window(window) {

	this->angle = glm::vec2(0);
	this->rotation = glm::vec3(0);
	this->pos = glm::vec3(2, 2, -2);

	this->sensivity = sensitivity;
	this->speed = speed;

	getCursorPos(&cursor.x, &cursor.y);
	this->last_frame = glfwGetTime();
}

void Camera::move(const glm::vec3& pos) {
	this->pos = pos;
}

void Camera::updateTime() {
	const double now = glfwGetTime();

	this->delta_time = now - this->last_frame;
	this->last_frame = now;
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
	return glm::lookAt(this->pos, this->pos + this->direction, glm::vec3(0.0f, -1.0f, 0.0f));
}



