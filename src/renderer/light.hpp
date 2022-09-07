#pragma once

#include <include/external.hpp>

enum struct LightType : int {
	Null   = 0,
	Point  = 1,
	Spot   = 2,
	Global = 3
};

// Warning: Light type (LightType) is stored as 
// float in the `w` component of diffuse_color
class Light {

	private:

		glm::vec4 diffuse_color;
		glm::vec4 specular_color;
		glm::vec4 position;
		glm::vec4 direction;
	
		// spot light
		float angle_inner;
		float angle_outer;

		// attenuation
		float linear;
		float quadratic;

	public:

		Light(LightType type);

		Light& setColor( const glm::vec3& color );
		Light& setColor( float r, float g, float b );
		Light& setDiffuseColor( const glm::vec3& color );
		Light& setDiffuseColor( float r, float g, float b );
		Light& setSpecularColor( const glm::vec3& color );
		Light& setSpecularColor( float r, float g, float b );
		Light& setAttenuation( float linear, float quadratic );
		Light& setAngle( float inner, float outer );
		Light& setPosition( const glm::vec3& position );
		Light& setDirection( const glm::vec3& direction );

		glm::vec4& getDiffuseColor();
		glm::vec4& getSpecularColor();
		glm::vec4& getPosition();

};

