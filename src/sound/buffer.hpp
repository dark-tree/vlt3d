#pragma once

#include "external.hpp"

class SoundBuffer {

	private:

		friend class SoundSource;

		READONLY ALuint al_buffer;
		READONLY const char* path;

		uint32_t format_of(uint32_t channels) {
			return (channels > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		}

	public:

		SoundBuffer(const char* path)
		: path(path) {
			alGenBuffers(1, &al_buffer);
			alCheckError("alGenBuffers");

			int channels;
			int samples;
			short* data;

			long count = stb_vorbis_decode_filename(path, &channels, &samples, &data);

			if (count == -1) {
				logger::error("Failed to load sound: '", path, "'!");
			} else {
				logger::info("Uploading sound: '", path, "', count: ", count, ", channels: ", channels, ", freq: ", samples);
			}

			uint32_t length = count * channels * sizeof(short);
			int format = format_of(channels);

			alBufferData(al_buffer, format, data, length, samples);
			alCheckError("alBufferData");

			if (format == AL_FORMAT_STEREO16) {
				logger::warn("Attenuation not supported for stereo sound: '", path, "'!");
			}

			// the documentation for STB vorbis is non-existent
			// i assume this is what should be done, other STB libraries typically had a
			// custom free function but for this one i couldn't find any
			free(data);
		}

		void close() {
			alDeleteBuffers(1, &al_buffer);
		}

		const char* getName() const {
			return path;
		}

};
