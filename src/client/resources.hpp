#pragma once

#include "external.hpp"
#include "buffer/atlas.hpp"
#include "buffer/font.hpp"
#include "render/view.hpp"
#include "shader/module.hpp"
#include "buffer/array.hpp"

class CommandBuffer;
class Device;
class Allocator;
class RenderSystem;

class ResourceManager {

	public:

		struct State {

			Device& device;
			SpriteArray array;
			Atlas atlas;
			Font font;

			Image atlas_image;
			ImageView atlas_view;
			ImageSampler atlas_sampler;

			Image array_image;
			ImageView array_view;
			ImageSampler array_sampler;

			ShaderModule vert_2d;
			ShaderModule vert_3d;
			ShaderModule vert_terrain;
			ShaderModule vert_blit;
			ShaderModule frag_terrain;
			ShaderModule frag_tint;
			ShaderModule frag_compose;
			ShaderModule frag_ssao;

			State(RenderSystem& system, TaskQueue& queue, CommandRecorder& recorder);
			~State();

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
		void reload(RenderSystem& system, TaskQueue& queue, CommandBuffer pool);

		/// close all resource and load nothing, expects that none of the assets are in use
		void close();

};