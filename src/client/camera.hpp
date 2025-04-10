#pragma once

#include "external.hpp"
#include "object.hpp"
#include "window/window.hpp"
#include "frustum.hpp"

class Camera : public WorldObject {

	private:

		Window& window;

		glm::vec2 angle;
		glm::vec2 cursor;
		glm::vec3 pos;
		glm::vec3 rotation;
		glm::vec3 direction;

		float sensitivity;
		float speed;
		double last_time;

		// debug things
		glm::mat4 view;
		bool locked = false;

		float getTimeDelta();
		glm::vec2 getMouseDelta();

	public:

		Camera(Window& window, float sensitivity = 0.2f, float speed = 15.0f);

		void move(const glm::vec3&	pos);
		void update();

		glm::vec3 getPosition() const override;
		glm::vec3 getDirection() const override;
		glm::vec3 getUp() const;
		glm::mat4 getView() const;
		Frustum getFrustum(glm::mat4& projection) const;

};
