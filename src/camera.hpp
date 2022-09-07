#pragma once

#include <core.hpp>

class Camera {

	private:
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
		static void getCursorPos( float* x, float* y );

	public:
		Camera( float sensivity = 0.2f, float speed = 15.0f );

		void move( const glm::vec3&	pos );
		void update();

		glm::vec3& getPosition();
		glm::vec3& getRotation();
		glm::mat4 getView();

};
