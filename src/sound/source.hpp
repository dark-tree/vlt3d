#pragma once

#include "external.hpp"
#include "volume.hpp"
#include "buffer.hpp"

class SoundSource {

	public:

		using Attachment = std::function<void(SoundSource&)>;

	private:

		friend class SoundSystem;

		READONLY ALuint al_source;
		READONLY const char* path;

		Attachment attachment;
		const SoundVolumes& sound_volumes;
		SoundGroup group = SoundGroup::MASTER;
		float gain;

		void flush() {
			alSourcef(al_source, AL_GAIN, gain * sound_volumes.get(group));
			alCheckError("alSourcef");
		}

	public:

		SoundSource(const SoundBuffer& buffer, const SoundVolumes& sound_volumes)
		: path(buffer.getName()), sound_volumes(sound_volumes) {
			alGenSources(1, &al_source);
			alCheckError("alGenSources");

			alSourcef(al_source, AL_REFERENCE_DISTANCE, 5);
			alSourcei(al_source, AL_BUFFER, buffer.al_buffer);
			alSource3f(al_source, AL_POSITION, 0, 0, 0);
			alCheckError("alSourcef");

			volume(1.0f);
		}

		~SoundSource() {
			alDeleteSources(1, &al_source);
			alCheckError("alDeleteSources");
		}

		const char* getName() const {
			return path;
		}

		bool shouldRemove() const {
			int state;
			alGetSourcei(al_source, AL_SOURCE_STATE, &state);

			return state == AL_STOPPED;
		}

		void update() {
			if (attachment) attachment(*this);
		}

	// play state management
	public:

		SoundSource& play() {
			flush();
			alSourcePlay(al_source);
			alCheckError("alSourcePlay");
			return *this;
		}

		SoundSource& pause() {
			alSourcePause(al_source);
			alCheckError("alSourcePause");
			return *this;
		}

		SoundSource& drop() {
			alSourceStop(al_source);
			alCheckError("alSourceStop");
			return *this;
		}

	// source properties
	public:

		SoundSource& attach(const Attachment& attachment) {
			this->attachment = attachment;
			return *this;
		}

		SoundSource& in(SoundGroup group) {
			this->group = group;
			return *this;
		}

		SoundSource& loop(bool value = true) {
			alSourcei(al_source, AL_LOOPING, value);
			alCheckError("alSourcei");
			return *this;
		}

		SoundSource& volume(float value) {
			this->gain = value;
			return *this;
		}

		SoundSource& pitch(float value) {
			alSourcef(al_source, AL_PITCH, value);
			alCheckError("alSourcef");
			return *this;
		}

		SoundSource& position(glm::vec3 value) {
			alSourcefv(al_source, AL_POSITION, glm::value_ptr(value));
			alCheckError("alSourcefv");
			return *this;
		}

		SoundSource& velocity(glm::vec3 value) {
			alSourcefv(al_source, AL_VELOCITY, glm::value_ptr(value));
			alCheckError("alSourcefv");
			return *this;
		}

		SoundSource& direction(glm::vec3 value) {
			alSourcefv(al_source, AL_DIRECTION, glm::value_ptr(value));
			alCheckError("alSourcefv");
			return *this;
		}

	// state getters
	public:

		int samples() const {
			int value;
			alGetSourcei(al_source, AL_SAMPLE_OFFSET, &value);
			alCheckError("alGetSourcei");

			return value;
		}

		float seconds() const {
			float value;
			alGetSourcef(al_source, AL_SEC_OFFSET, &value);
			alCheckError("alGetSourcef");

			return value;
		}

};
