
#include "skybox.hpp"

/// Returns Sun's Equatorial Coordinates over time
glm::vec2 Skybox::getSun(float hour, float day) const {

	// The hour angle represents how far the sun has moved from local solar noon
	// Solar noon is when the sun is highest in the sky (12:00)
	// Each hour away from solar noon moves the sun by SUN_SPEED radians
	const float hour_angle = (hour - 12.0f) * SUN_SPEED;

	// The declination is the angle between the sun and the celestial equator
	// It changes throughout the year as the Earth orbits the Sun due to the tilt of the earth
	// The declination changes sinusoidally over the year
	const float declination = AXIAL_TILT * sin(2.0f * M_PI * (day - 81.0f) / 365.0f);

	return {hour_angle, declination};
}

/// Translates Equatorial Coordinates to observer's Horizontal Coordinates
glm::vec2 Skybox::toHorizontal(glm::vec2 equatorial, float latitude) const {

	const float hour_angle = equatorial.x;
	const float declination = equatorial.y;

	// precompute reused values
	const float sin_lat = sin(latitude);
	const float cos_lat = cos(latitude);
	const float sin_dec = sin(declination);
	const float cos_dec = cos(declination);

	// The altitude is the angle of the sun above or below the horizon
	// It depends on the declination of the sun, the observer's latitude, and the hour angle
	float sin_alt = sin_dec * sin_lat + cos_dec * cos_lat * cos(hour_angle);
	float altitude = asin(sin_alt);

	if (altitude < -M_PI / 2.0f) {
		altitude = -M_PI / 2.0f;
	} else if (altitude > M_PI / 2.0f) {
		altitude = M_PI / 2.0f;
	}

	float cos_alt = cos(altitude);

	// The azimuth is the compass direction of the sun, measured from North
	// It tells us where the sun is located horizontally
	float sin_azimuth = -cos_dec * sin(hour_angle) / cos_alt;
	float cos_azimuth = (sin_dec - sin_lat * sin_alt) / (cos_lat * cos_alt);
	float azimuth = atan2(sin_azimuth, cos_azimuth);

	if (azimuth < 0) {
		azimuth += 2.0f * M_PI;
	}

	return glm::vec2(azimuth, altitude);
}

void Skybox::drawSphere(ImmediateRenderer& immediate, glm::vec3 pos, float radius, int longs, int lats, BakedSprite sprite) const {

	float us = sprite.u1;
	float vs = sprite.v1;
	float ud = sprite.u2 - sprite.u1;
	float vd = sprite.v2 - sprite.v1;

	for (int lat = 0; lat < lats; ++lat) {
		float lat0 = M_PI * (-0.5 + (float)(lat) / lats);
		float lat1 = M_PI * (-0.5 + (float)(lat + 1) / lats);
		float sinLat0 = sin(lat0);
		float cosLat0 = cos(lat0);
		float sinLat1 = sin(lat1);
		float cosLat1 = cos(lat1);

		for (int lon = 0; lon < longs; ++lon) {
			float lon0 = 2 * M_PI * (float)(lon) / longs;
			float lon1 = 2 * M_PI * (float)(lon + 1) / longs;
			float sinLon0 = sin(lon0);
			float cosLon0 = cos(lon0);
			float sinLon1 = sin(lon1);
			float cosLon1 = cos(lon1);

			// Vertex coordinates for the first triangle
			float x0 = radius * cosLat0 * cosLon0;
			float y0 = radius * sinLat0;
			float z0 = radius * cosLat0 * sinLon0;
			float u0 = (float)lon / longs;
			float v0 = (float)lat / lats;
			immediate.drawVertex(pos.x + x0, pos.y + y0, pos.z + z0, us + u0 * ud, vs + v0 * vd);

			float x2 = radius * cosLat1 * cosLon1;
			float y2 = radius * sinLat1;
			float z2 = radius * cosLat1 * sinLon1;
			float u2 = (float)(lon + 1) / longs;
			float v2 = (float)(lat + 1) / lats;
			immediate.drawVertex(pos.x + x2, pos.y + y2, pos.z + z2, us + u2 * ud, vs + v2 * vd);

			float x1 = radius * cosLat1 * cosLon0;
			float y1 = radius * sinLat1;
			float z1 = radius * cosLat1 * sinLon0;
			float u1 = (float)lon / longs;
			float v1 = (float)(lat + 1) / lats;
			immediate.drawVertex(pos.x + x1, pos.y + y1, pos.z + z1, us + u1 * ud, vs + v1 * vd);

			// Vertex coordinates for the second triangle
			immediate.drawVertex(pos.x + x2, pos.y + y2, pos.z + z2, us + u2 * ud, vs + v2 * vd);
			immediate.drawVertex(pos.x + x0, pos.y + y0, pos.z + z0, us + u0 * ud, vs + v0 * vd);

			float x3 = radius * cosLat0 * cosLon1;
			float y3 = radius * sinLat0;
			float z3 = radius * cosLat0 * sinLon1;
			float u3 = (float)(lon + 1) / longs;
			float v3 = (float)lat / lats;
			immediate.drawVertex(pos.x + x3, pos.y + y3, pos.z + z3, us + u3 * ud, vs + v3 * vd);
		}
	}
}

glm::vec3 Skybox::getSunPos(float observer_latitude) const {

	glm::vec2 equ = getSun((float) glfwGetTime() / 2.0f, glfwGetTime() / 2);
	glm::vec2 hor = toHorizontal(equ, observer_latitude);

	float longitude = hor.x;
	float latitude = hor.y;

	float equator_x = cos(longitude);
	float equator_z = sin(longitude);
	float y = sin(latitude);
	float multiplier = cos(latitude);
	float x = multiplier * equator_x;
	float z = multiplier * equator_z;

	// inverted to align with world directions as defined in Direction class
	return glm::normalize(glm::vec3 {z, y, x});

}

void Skybox::draw(ImmediateRenderer& immediate, Camera& camera) const {

	glm::vec3 observer = camera.getPosition();
	float radius = 500;

	// 1. DRAW STARS

	Random random {100};
	BakedSprite sprites[3];

	sprites[0] = immediate.getSprite("star-1");
	sprites[1] = immediate.getSprite("star-2");
	sprites[2] = immediate.getSprite("star-3");

	for (int i = 0; i < 300; i ++) {
		float x = random.gaussianFloat(0.0f, 1.0f);
		float y = random.gaussianFloat(0.0f, 1.0f);
		float z = random.gaussianFloat(0.0f, 1.0f);

		immediate.drawSprite(observer + glm::normalize(glm::vec3 {x, y, z}) * radius, 4, 4, sprites[random.uniformInt(2)]);
	}

	// 2. DRAW SKY

	immediate.setTint(255, 255, 255, 50);
	drawSphere(immediate, observer, radius, 10, 10, immediate.getSprite("skybox-simple"));

	// 3. DRAW SUN

	glm::vec3 sun = getSunPos(camera.getPosition().x / 100);

	immediate.setTint(255, 255, 255, 255);
	immediate.drawSprite(observer + sun * 450.0f, 64, 64, immediate.getSprite("sun"));

}