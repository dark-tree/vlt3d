#pragma once

#include "external.hpp"
#include "object.hpp"
#include "window/window.hpp"

class Camera : public WorldObject {

	private:

		Window& window;

		glm::vec2 angle;
		glm::vec2 cursor;
		glm::vec3 pos;
		glm::vec3 rotation;
		glm::vec3 direction;

		float sensivity;
		float speed;

		double last_frame;
		double delta_time;

		void updateTime();
		void getCursorPos(float* x, float* y);

	public:

		Camera(Window& window, float sensitivity = 0.2f, float speed = 15.0f);

		void move(const glm::vec3&	pos);
		void update();

		glm::vec3 getPosition() const override;
		glm::vec3 getDirection() const override;
		glm::vec3 getUp() const;
		glm::mat4 getView() const;

};
