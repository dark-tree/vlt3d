
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

struct Frame {

	CommandBuffer buffer;
	Semaphore image_available_semaphore;
	Semaphore render_finished_semaphore;
	Fence in_flight_fence;

	UBO data;
	Buffer ubo;
	MemoryMap map;
	DescriptorSet set;

	Frame(Allocator& allocator, const CommandPool& pool, const Device& device, DescriptorSet descriptor, const ImageSampler& sampler)
	: buffer(pool.allocate()), image_available_semaphore(device.semaphore()), render_finished_semaphore(device.semaphore()), in_flight_fence(device.fence(true)) {

		BufferInfo buffer_builder {sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};
		buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

		ubo = allocator.allocateBuffer(buffer_builder);
		map = ubo.access().map();
		set = descriptor;

		descriptor.buffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ubo, sizeof(UBO));
		descriptor.sampler(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sampler);
	}

	void wait() {
		in_flight_fence.lock();
	}

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

		GraphicsPipeline pipeline_3d_mix;
		GraphicsPipeline pipeline_3d_tint;
		GraphicsPipeline pipeline_2d_tint;

		int concurrent;

		// the index of the next frame to render this value is kept within
		// the size of the frames vector (the vector is used as a ring buffer)
		int index;

		// a ring-buffer line holder for the per frame states, utilized for concurrent
		// frame rendering (each frame can be drawn at the same time)
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

	public:

		RenderSystem(TaskPool& pool, Window& window, int concurrent)
		: window(window), concurrent(concurrent) {

			///
			/// Phase 1
			/// this step is only ever executed once
			///

			// instance configuration
			InstanceBuilder builder;
			builder.addApplicationInfo("My Funny Vulkan Application");
			builder.addValidationLayer("VK_LAYER_KHRONOS_validation").orFail();
			builder.addDebugMessenger();

			// instance and surface creation, and device selection
			instance = builder.build();
			surface = instance.createSurface(window);
			DeviceBuilder device_builder = pickDevice(instance, surface);

			// device configuration
			QueueInfo graphics_ref = device_builder.addQueue(QueueType::GRAPHICS, 1);
			QueueInfo transfer_ref = device_builder.addQueue(QueueType::TRANSFER, 1);
			QueueInfo presentation_ref = device_builder.addQueue(surface, 1);
			device_builder.addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME).orFail();

			// enable some additional features
			device_builder.features.enableFillModeNonSolid().orFail();
			device_builder.features.enableWideLines().orFail();

			// device and queue creation
			device = device_builder.create();
			graphics_queue = device.get(graphics_ref, 0);
			transfer_queue = device.get(transfer_ref, 0);
			presentation_queue = device.get(presentation_ref, 0);

			// create command pools
			graphics_pool = CommandPool::build(device, graphics_ref, false);
			transient_pool = CommandPool::build(device, transfer_ref, true);

			// get the VMA allocator ready
			allocator = Allocator {device, instance};

			///
			/// Phase 2
			/// this step needs to be more or less repeated every time the window size changes
			///

			createSwapchain();
			createRenderPass(swapchain);
			createFramebuffers(render_pass);

			///
			/// Phase 3
			/// this step will need to be repeated each time the resources are reloaded
			///

			Compiler compiler;
			std::shared_future<ShaderModule> vert_2d = pool.defer([&] { return compiler.compileFile("assets/shaders/vert_2d.glsl", Kind::VERTEX).create(device); }).share();
			std::shared_future<ShaderModule> vert_3d = pool.defer([&] { return compiler.compileFile("assets/shaders/vert_3d.glsl", Kind::VERTEX).create(device); }).share();
			std::shared_future<ShaderModule> frag_mix = pool.defer([&] { return compiler.compileFile("assets/shaders/frag_mix.glsl", Kind::FRAGMENT).create(device); }).share();
			std::shared_future<ShaderModule> frag_tint = pool.defer([&] { return compiler.compileFile("assets/shaders/frag_tint.glsl", Kind::FRAGMENT).create(device); }).share();

			logger::debug("Resource reload took: ", Timer::of([&] {
				Fence fence = device.fence();
				CommandBuffer buffer = transient_pool.allocate();
				assets.reload(device, allocator, buffer);
				buffer.submit().unlocks(fence).done(transfer_queue);

				fence.wait();
				fence.close();
				buffer.close();
			}).milliseconds(), "ms");

			// Create this thing
			DescriptorSetLayout descriptor_layout = DescriptorSetLayoutBuilder::begin()
				.descriptor(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
				.descriptor(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.done(device);

			// 3D binding layout
			BindingLayout binding_3d = BindingLayoutBuilder::begin()
				.attribute(0, VK_FORMAT_R32G32B32_SFLOAT)
				.attribute(1, VK_FORMAT_R32G32_SFLOAT)
				.attribute(2, VK_FORMAT_R32_UINT)
				.done();

			// 2D binding layout
			BindingLayout binding_2d = BindingLayoutBuilder::begin()
				.attribute(0, VK_FORMAT_R32G32_SFLOAT)
				.attribute(1, VK_FORMAT_R32G32_SFLOAT)
				.attribute(2, VK_FORMAT_R32_UINT)
				.done();

			pipeline_3d_mix = GraphicsPipelineBuilder::of(device)
				.withDynamics(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR)
				.withRenderPass(render_pass)
				.withShaders(vert_3d, frag_mix)
				.withDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true, true)
				.withBindingLayout(binding_3d)
				.withDescriptorSetLayout(descriptor_layout)
				.build("3D Mixed");

			pipeline_3d_tint = GraphicsPipelineBuilder::of(device)
				.withDynamics(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR)
				.withRenderPass(render_pass)
				.withShaders(vert_3d, frag_tint)
				.withDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true, true)
				.withBlendMode(BlendMode::ENABLED)
				.withBlendAlphaFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
				.withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
				.withBindingLayout(binding_3d)
				.withDescriptorSetLayout(descriptor_layout)
				.build("3D Tinted");

			pipeline_2d_tint = GraphicsPipelineBuilder::of(device)
				.withDynamics(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR)
				.withRenderPass(render_pass)
				.withShaders(vert_2d, frag_tint)
				.withBlendMode(BlendMode::ENABLED)
				.withBlendAlphaFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
				.withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
				.withBindingLayout(binding_2d)
				.withDescriptorSetLayout(descriptor_layout)
				.build("2D Tinted");

			DescriptorPool descriptor_pool = DescriptorPoolBuilder::begin()
				.add(concurrent, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				.add(concurrent, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				.done(device, concurrent);

			for (int i = 0; i < concurrent; i ++) {
				frames.emplace_back(allocator, graphics_pool, device, descriptor_pool.allocate(descriptor_layout), assets.getAtlasSampler());
			}
		}

		Framebuffer& acquireFramebuffer() {
			uint32_t image_index;
			if (swapchain.getNextImage(getFrame().image_available_semaphore, &image_index).mustReplace()) {
				recreateSwapchain();
			}

			return framebuffers[image_index];
		}

		void presentFramebuffer(Framebuffer& framebuffer) {
			if (swapchain.present(presentation_queue, getFrame().render_finished_semaphore, framebuffer.index).mustReplace()) {
				recreateSwapchain();
			}
		}

		Frame& getFrame() {
			return frames[index];
		}

		void nextFrame() {
			index = (index + 1) % concurrent;
		}

		/**
		 * Wait (block) for all pending operations on all queues are finished
		 */
		void wait() {
			device.wait();
		}

};