#pragma once

#include "texture.hpp"

struct Material {

	float shininess;
	Texture* specular_map;
	Texture* emissive_map;
	Texture* diffuse_map;

};

