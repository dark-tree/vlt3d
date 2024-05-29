#pragma once

#include "external.hpp"

class SoundListener {

	public:

		// the position of the virtual listener
		SoundListener& position(glm::vec3 value) {
			alListenerfv(AL_POSITION, glm::value_ptr(value));
			alCheckError("alListenerfv");
			return *this;
		}

		// the facing of the virtual listener, see https://stackoverflow.com/a/7866395
		SoundListener& facing(glm::vec3 at, glm::vec3 up) {
			float vectors[] = {at.x, at.y, at.z, up.x, up.y, up.z};
			alListenerfv(AL_ORIENTATION, vectors);
			alCheckError("alListenerfv");
			return *this;
		}

		// the velocity of the virtual listener
		SoundListener& velocity(glm::vec3 value) {
			alListenerfv(AL_VELOCITY, glm::value_ptr(value));
			alCheckError("alListenerfv");
			return *this;
		}

		// the "master volume"
		SoundListener& gain(float value) {
			alListenerf(AL_GAIN, value);
			alCheckError("alListenerf");
			return *this;
		}

};
