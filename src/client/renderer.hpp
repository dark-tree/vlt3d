
#pragma once

#include "sync/semaphore.hpp"
#include "sync/fence.hpp"
#include "command/buffer.hpp"
#include "command/pool.hpp"
#include "descriptor/pool.hpp"
#include "setup/swapchain.hpp"
#include "shader/compiler.hpp"
#include "util/thread/delegator.hpp"
#include "resources.hpp"
#include "util/arena.hpp"

struct SceneUniform {
	glm::mat4 mvp;
	glm::mat4 view;
	glm::mat4 normal;
};

struct AmbientOcclusionUniform {
	glm::vec4 samples[64];
	glm::vec2 noise_scale;
};

class Frame {

	private:

		enum Sequence : uint8_t {
			INITIAL,
			FIRST,
			SUBSEQUENT
		};

		friend class RenderSystem;

		/// this needs to be before any BasicBuffer so that the internal mutex is ready
		/// before the BasicBuffer constructor runs as it uses `system.defer()`
		TaskQueue queue;

		/// used to identify if this frame is being rendered for the first time or not
		/// with INITIAL and FIRST corresponding to being used for the first time
		Sequence sequence;

		/// Small buffer used to store uniform variables
		/// after writing data to `uniforms` call uploadUniformBuffer()
		Buffer uniform_buffer;

		/// The memory map backing the uniform_buffer
		/// Uniform buffer doesn't use staging so this is just some host memory map
		MemoryMap uniform_map;

	public:

		CommandBuffer buffer;

		/// the buffers used by the ImmediateRender class
		/// we could maybe merge them into a singe one for performance
		BasicBuffer immediate_2d, immediate_3d;

		/// this semaphore will be unlocked once the frambuffer returned from
		/// `RenderSystem::acquireFramebuffer()` is actually ready to be written to, see `CommandSubmitter::awaits()`
		Semaphore available_semaphore;

		/// this should be unlocked by the frame consumer using `CommandSubmitter::unlocks()`
		/// as `RenderSystem::presentFramebuffer()` waits for this semaphore to be unlocked
		Semaphore finished_semaphore;

		/// this should be unlocked by the frame consumer using `CommandSubmitter::unlocks()`
		/// and waited on using `wait()` before starting the rendering of a frame, it is used to keep CPU and GPU in sync
		Fence flight_fence;

		SceneUniform uniforms;
		DescriptorSet set_0, set_1, set_2, set_3;

		QueryPool timestamp_query;
		QueryPool occlusion_query;

	public:

		Frame(RenderSystem& system, const CommandPool& pool, const Device& device, const ImageSampler& atlas_sampler);

		/**
		 * This class is fully managed by the RenderSystem so it uses
		 * a destructor not a standard close() method
		 */
		~Frame();

		/**
		 * Wait before starting to render this frame
		 * until the last frame with this index has been completed
		 * (this is needed to keep the CPU from "running away" from the GPU).
		 */
		void wait();

		/**
		 * Execute all pending frame tasks, this usually is used
		 * for running cleanup hooks
		 */
		void execute();

		/**
		 * Check if this frame is being used for the first time
		 */
		bool first() const;

		/**
		 * Write data into the mapped memory of the shared uniform buffer
		 */
		void flushUniformBuffer();

		/**
		 * Returns a queue for tasks to be executed one frame cycle later
		 */
		TaskQueue& getDelegator();

};

class RenderSystem {

	public:

		TaskPool pool {3};
		MailboxTaskDelegator presenter {pool};

		Window& window;
		WindowSurface surface;

		Instance instance;
		Device device;
		Allocator allocator;

		Queue graphics_queue;
		Queue transfer_queue;
		Queue presentation_queue;

		CommandPool graphics_pool;
		CommandPool transient_pool;

		// SSAO
		Image ssao_noise_image;
		ImageView ssao_noise_view;
		ImageSampler ssao_noise_sampler;
		Buffer ssao_uniform_buffer;

		// Chunk Occlusion
		Buffer chunk_box;
		Buffer chunk_predicates;
		QueryPool predicate_query;
		LinearArena predicate_allocator;

		Attachment attachment_color;
		Attachment attachment_depth;
		Attachment attachment_albedo;
		Attachment attachment_normal;
		Attachment attachment_position;
		Attachment attachment_ambience;

		RenderPass terrain_pass;
		RenderPass conditional_pass;
		RenderPass ssao_pass;
		RenderPass lighting_pass;

		GraphicsPipeline pipeline_terrain;
		GraphicsPipeline pipeline_conditional;
		GraphicsPipeline pipeline_3d_tint;
		GraphicsPipeline pipeline_2d_tint;
		GraphicsPipeline pipeline_ssao;
		GraphicsPipeline pipeline_compose;
		GraphicsPipeline pipeline_occlude;

		Swapchain swapchain;
		std::vector<Framebuffer> framebuffers;
		Framebuffer ssao_framebuffer;
		Framebuffer terrain_framebuffer;
		Framebuffer conditional_framebuffer;

		ResourceManager assets;

		DescriptorSetLayout geometry_descriptor_layout;
		DescriptorSetLayout ssao_descriptor_layout;
		DescriptorSetLayout lighting_descriptor_layout;

		BindingLayout binding_terrain;
		BindingLayout binding_occlude;
		BindingLayout binding_3d;
		BindingLayout binding_2d;
		DescriptorPool descriptor_pool;

		// TODO this is a mess
		PushConstantLayout constant_layout;
		PushConstant push_constant;

		PushConstantLayout constant_layout_occlude;
		PushConstant push_constant_occlude;

	private:

		/// the number of concurrent frames, this value should no be larger then 4-5 to no cause input delay
		/// setting it to 1 effectively disables concurrent frames
		int concurrent;

		/// the index of the next frame to render, this value is kept within
		/// the size of the frames vector (the vector is used as a ring buffer)
		int index;

		/// a ring-buffer line holder for the per frame states, utilized for concurrent
		/// frame rendering (the CPU can "render ahead" of the GPU)
		std::vector<Frame> frames;

	private:

		/**
		 * Picks a device that has all the features that we will need
		 * if no device is found throws an exception
		 */
		DeviceBuilder pickDevice(Instance& instance, WindowSurface& surface);

		/**
		 * Create a swapchain that matches our window, needs to be called before we can create
		 * a render pass or frame buffer, and recalled when the window changes, see `acquireFramebuffer()`
		 */
		void createSwapchain();

		/**
		 * Creates the vulkan render pass, this MAY need to be called when the
		 * swapchain is recreated, depending on the format picked by `createSwapchain()`
		 */
		void createRenderPass();

		/**
		 * Populates the framebuffer array, needs to be called after the swapchain is recreated
		 * this function depends on render passes as the created framebuffers need to be compatible with them
		 */
		void createFramebuffers();

		/**
		 * Called from `acquireFramebuffer()` and `presentFramebuffer()` when the current swapchain is
		 * no longer compatible with the window (for example when the size changed)
		 */
		void recreateSwapchain();

		/**
		 * This method creates all the pipelines used by the game
		 * it will be called every time a resource reload or swapchain recreation occurs
		 */
		void createPipelines();

		/**
		 *
		 */
		void closeFrames();

		/**
		 *
		 */
		void createFrames();

		/**
		 *
		 */
		void initScreenSpaceAmbientOcclusion();

		/**
		 *
		 */
		void initChunkOcclusion();

	public:

		/// Only one instance of render system should ever be created
		RenderSystem(Window& window, int concurrent);

		/// Reloads all game assets from disc and recreates are the necessary data structures
		void reloadAssets();

		/// Get the screen's framebuffer reference
		Framebuffer& acquireScreenFramebuffer();

		/// Queue the given framebuffer for rendering on the screen
		void presentScreenFramebuffer(const Framebuffer& framebuffer);

		/// Get a reference to the current frame state
		Frame& getFrame();

		/// Advanced to the next frame, must be called AFTER `presentFramebuffer()`
		void nextFrame();

		/// Defer some task until the next frame of the same index, useful for running cleanup hooks
		void defer(const Task& task);

		/// Wait for all pending operations on all queues are finished
		void wait();

		/// Free are resources managed by this render system
		void close();

};