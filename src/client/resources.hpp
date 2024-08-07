#pragma once

#include "external.hpp"
#include "buffer/atlas.hpp"
#include "buffer/font.hpp"
#include "render/view.hpp"
#include "shader/module.hpp"

class CommandBuffer;
class Device;
class Allocator;

class ResourceManager {

	public:

		struct State {

			Atlas atlas;
			Font font;
			ImageSampler sampler;

			ShaderModule vert_2d;
			ShaderModule vert_3d;
			ShaderModule vert_terrain;
			ShaderModule frag_terrain;
			ShaderModule frag_tint;

			State(Device& device, Allocator& allocator, CommandRecorder& recorder);

		};

		std::atomic_flag loading = ATOMIC_FLAG_INIT;
		State* state = nullptr;

		/// make the given state current, and frees the old one
		void replace(State* state);

	public:

		const Atlas& getAtlas() const;
		const Font& getFont() const;
		const ImageSampler& getAtlasSampler() const;

		/**
		 * Blocks until the new assets are ready
		 *
		 * @todo
		 * in the future make this returns
		 * some sort of a progress tracker
		 * object and show a loading screen
		 */
		void reload(Device& device, Allocator& allocator, CommandBuffer pool);

};