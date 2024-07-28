
#pragma once

#include "sync/semaphore.hpp"
#include "sync/fence.hpp"
#include "command/buffer.hpp"
#include "command/pool.hpp"
#include "descriptor/pool.hpp"
#include "setup/swapchain.hpp"
#include "shader/compiler.hpp"
#include "util/threads.hpp"
#include "resources.hpp"

struct UBO {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class Frame {

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

		UBO data;
		Buffer ubo;
		MemoryMap map;
		DescriptorSet set;

	public:

		Frame(Allocator& allocator, const CommandPool& pool, const Device& device, DescriptorSet descriptor, const ImageSampler& sampler);

		/**
		 * This class is fully managed by the RenderSystem so it uses
		 * a destructor not a standard close() method
		 */
		~Frame();

		/**
		 * Wait before starting to render this frame
		 * until the last frame with this index has been completed,
		 * this is too keep the CPU from "running away" from the GPU
		 */
		void wait();

};

class RenderSystem {

	public:

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

		Swapchain swapchain;
		RenderPass render_pass;
		Image depth_image;
		ImageView depth_view;
		std::vector<Framebuffer> framebuffers;

		ResourceManager assets;

		DescriptorSetLayout descriptor_layout;
		BindingLayout binding_3d;
		BindingLayout binding_2d;
		GraphicsPipeline pipeline_3d_mix;
		GraphicsPipeline pipeline_3d_tint;
		GraphicsPipeline pipeline_2d_tint;

		DescriptorPool descriptor_pool;

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
		void createRenderPass(Swapchain& surface);

		/**
		 * Populates the framebuffer array, needs to be called after the swapchain is recreated
		 * this function depends on render pass as the created framebuffers need to be compatible with it
		 */
		void createFramebuffers(RenderPass& pass);

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

	public:

		/// Only one instance of render system should ever be created
		RenderSystem(Window& window, int concurrent);

		/// Reloads all game assets from disc and recreates are the necessary data structures
		void reloadAssets();

		/// Get the screen's framebuffer reference
		Framebuffer& acquireFramebuffer();

		/// Queue the given framebuffer for rendering on the screen
		void presentFramebuffer(Framebuffer& framebuffer);

		/// Get a reference to the current frame state
		Frame& getFrame();

		/// Advanced to the next frame, must be called AFTER `presentFramebuffer()`
		void nextFrame();

		/// Wait for all pending operations on all queues are finished
		void wait();

};