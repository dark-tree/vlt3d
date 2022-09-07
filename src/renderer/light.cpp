#include "light.hpp"

Light::Light(LightType type) {
	this->diffuse_color.w = (float) type;
}

Light& Light::setColor( const glm::vec3& color ) {
	setDiffuseColor( color );
	setSpecularColor( color );
	return *this;
}

Light& Light::setColor( float r, float g, float b ) {
	setDiffuseColor( r, g, b );
	setSpecularColor( r, g, b );
	return *this;
}

Light& Light::setDiffuseColor( const glm::vec3& color ) {
	this->diffuse_color = glm::vec4(color, this->diffuse_color.w);
	return *this;
}

Light& Light::setDiffuseColor( float r, float g, float b ) {
	this->diffuse_color = glm::vec4(r, g, b, this->diffuse_color.w);
	return *this;
}

Light& Light::setSpecularColor( const glm::vec3& color ) {
	this->specular_color = glm::vec4(color, 0.0f);
	return *this;
}

Light& Light::setSpecularColor( float r, float g, float b ) {
	this->specular_color = glm::vec4(r, g, b, 0.0f);
	return *this;
}

Light& Light::setAttenuation( float linear, float quadratic ) {
	this->linear = linear;
	this->quadratic = quadratic;
	return *this;
}

Light& Light::setAngle( float inner, float outer ) {
	this->angle_inner = inner;
	this->angle_outer = outer;
	return *this;
}

Light& Light::setPosition( const glm::vec3& position ) {
	this->position = glm::vec4(position, 0.0f);
	return *this;
}

Light& Light::setDirection( const glm::vec3& direction ) {
	this->direction = glm::vec4(direction, 0.0f);
	return *this;
}

glm::vec4& Light::getDiffuseColor() {
	return this->diffuse_color;
}

glm::vec4& Light::getSpecularColor() {
	return this->specular_color;
}

glm::vec4& Light::getPosition() {
	return this->position;
}

