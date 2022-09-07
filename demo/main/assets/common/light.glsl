
#define NULL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2
#define GLOBAL_LIGHT 3

struct Material {
	sampler2D specular;
	sampler2D emissive;
	float shininess;
};

struct Light {

	vec3 diffuse_color; float type;
	vec3 specular_color; float p1;
	vec3 position; float p2;
	vec3 direction; float p3;
	
	// spotlight cutoff angle
	float angle_inner;
	float angle_outer;

	// attenuation
	float linear;
	float quadratic;

};

vec3 getPhongLight( Light light, vec3 direction, vec3 sp, vec3 normal, vec3 position, vec3 camera, float shininess, float multiplier ) {

	// ambient light
   	vec3 ambient = 0.15f * vec3(light.diffuse_color) * multiplier;
  	
	// diffuse light
	float diff = max(dot(normal, direction), 0.0f);
	vec3 diffuse = diff * vec3(light.diffuse_color) * multiplier;
    
	// specular light
	vec3 view_direction = normalize(camera - position);
	vec3 reflect_direction = reflect(-direction, normal);
	float spec = pow(max(dot(view_direction, reflect_direction), 0.0), shininess);
	vec3 specular = sp * spec * vec3(light.specular_color) * multiplier;

	return ambient + diffuse + specular;

}

vec3 getLight( Light light, vec3 sp, vec3 normal, vec3 position, vec3 camera, float shininess ) {

	float multiplier;
	vec3 direction;

	if( light.type == POINT_LIGHT ) {

		// attenuation
		float dist = length(light.position - position);
		float attenuation = 1.0f / (1.0f + light.linear * dist + light.quadratic * (dist * dist)); 

		direction = normalize(light.position - position);
		multiplier = attenuation;

	}

	else if( light.type == SPOT_LIGHT ) {
		
		direction = normalize(light.position - position);

		// cutoff angle
		float theta = dot(direction, normalize(-light.direction)); 
		float epsilon = (light.angle_inner - light.angle_outer);
		float intensity = clamp((theta - light.angle_outer) / epsilon, 0.0, 1.0);
		
		// attenuation
		float dist = length(light.position - position);
		float attenuation = 1.0f / (1.0f + light.linear * dist + light.quadratic * (dist * dist));

		multiplier = attenuation * intensity;

	}

	else if( light.type == GLOBAL_LIGHT ) {

		direction = normalize(-light.direction);
		multiplier = 1;

	}

	return getPhongLight(light, direction, sp, normal, position, camera, shininess, multiplier);

}

