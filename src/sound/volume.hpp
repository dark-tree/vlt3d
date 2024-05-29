#pragma once

#include "external.hpp"
#include "group.hpp"

class SoundVolumes {

	private:

		std::unordered_map<SoundGroup, float> volumes;

	public:

		float get(SoundGroup group) const {
			if (group == SoundGroup::MASTER) {
				return 1.0f;
			}

			return volumes.at(group);
		}

		void set(SoundGroup group, float value) {
			volumes[group] = value;
		}

};
