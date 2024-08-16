#pragma once

#include "external.hpp"
#include "debug.hpp"
#include "buffer.hpp"
#include "source.hpp"
#include "listener.hpp"
#include "volume.hpp"
#include "group.hpp"

class SoundSystem {

	private:

		READONLY ALCdevice* al_device;
		READONLY ALCcontext* al_context;

		std::list<std::unique_ptr<SoundSource>> sources;
		SoundVolumes sound_volumes;
		SoundListener sound_listener;

		SoundSource& add(std::unique_ptr<SoundSource>&& source) {
			sources.push_back(std::move(source));
			return *sources.back();
		}

	public:

		SoundSystem() {
			this->al_device = alcOpenDevice(nullptr);

			if (al_device == nullptr) {
				logger::fatal("Sound system failed to start, unable to open device!");
			}

			this->al_context = alcCreateContext(al_device, nullptr);

			if (al_context == nullptr) {
				logger::fatal("Sound system failed to start, unable to create context!");
			}

			alcMakeContextCurrent(al_context);
			alCheckError("alcMakeContextCurrent");

			glm::vec3 origin {0, 0, 0};
			sound_listener.position(origin).velocity(origin).gain(1.0f);

			logger::info("Sound system engaged");
		}

		~SoundSystem() {
			// C++ will first deconstruct SoundSystem and only then the sound sources which causes errors
			// as the context is already gone, so clear the array first to force the correct order
			sources.clear();

			alcMakeContextCurrent(nullptr);
			alcDestroyContext(al_context);
			alcCloseDevice(al_device);
		}

		/**
		 * Perform sound system book-keeping
		 */
		void update() {
			auto iter = sources.begin();

			while (iter != sources.end()) {
				bool remove = (*iter)->shouldRemove();

				if (remove) {
					iter = sources.erase(iter);
				} else {
					(*iter)->update();
					iter ++;
				}
			}
		}

		/**
		 * Create a new sound source from the buffer and returns it
		 */
		SoundSource& add(const SoundBuffer& buffer) {
			return add(std::make_unique<SoundSource>(buffer, sound_volumes));
		}

		/**
		 * Get the volume controller object
		 */
		SoundVolumes& getVolume() {
			return sound_volumes;
		}

		/**
		 * Get the sound listener object
		 */
		SoundListener& getListener() {
			return sound_listener;
		}

		/**
		 * Stop all sounds in the given group
		 */
		void stop(SoundGroup group = SoundGroup::MASTER) {
			for (auto& source : sources) {
				if (source->group == group || group == SoundGroup::MASTER) {
					source->drop();
				}
			}
		}

		/**
		 * Update volumes of all currently playing sounds
		 */
		void flush() {
			for (auto& source : sources) {
				source->flush();
			}
		}

};
