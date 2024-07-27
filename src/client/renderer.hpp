
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
};

class RenderSystem {

	public:

		Window& window;
		WindowSurface surface;

		Instance instance;
		Device device;
		Allocator allocator;

		// TODO merge the VkQueue and QueueInfo into one class, we often nerd both
		VkQueue graphics_queue; QueueInfo graphics_ref;
		VkQueue transfer_queue; QueueInfo transfer_ref;
		VkQueue presentation_queue; QueueInfo presentation_ref;

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

		void createRenderPass(Swapchain& surface) {

			VkFormat format = surface.vk_surface_format.format;
			RenderPassBuilder builder;

			builder.addAttachment(format, VK_SAMPLE_COUNT_1_BIT)
				.input(ColorOp::CLEAR, StencilOp::IGNORE, VK_IMAGE_LAYOUT_UNDEFINED)
				.output(ColorOp::STORE, StencilOp::IGNORE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
				.next();

			builder.addAttachment(VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT)
				.input(ColorOp::CLEAR, StencilOp::IGNORE, VK_IMAGE_LAYOUT_UNDEFINED)
				.output(ColorOp::IGNORE, StencilOp::IGNORE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
				.next();

			builder.addDependency()
				.input(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0)
				.output(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
				.next();

			builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
				.addColor(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
				.addDepth(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
				.next();

			render_pass = builder.build(device);

		}

		/**
		 * Pick a device that has all the features that we need
		 */
		DeviceBuilder pickDevice(Instance& instance, WindowSurface& surface) {
			for (DeviceInfo& device : instance.getDevices()) {
				if (device.getQueueFamily(QueueType::GRAPHICS) && device.getQueueFamily(surface) && device.hasSwapchain(surface)) {
					logger::info("Selected device: ", device.getProperties().deviceName);
					return device.builder();
				}
			}

			throw Exception {"No viable Vulkan device found!"};
		}

		void createSwapchain() {

			// swapchain information gathering
			SwapchainInfo info {device, surface};
			auto extent = info.getExtent(window);
			auto images = info.getImageCount();
			auto transform = info.getTransform();

			VkSurfaceFormatKHR selected = info.getFormats()[0];
			bool matched = false;

			for (auto& format : info.getFormats()) {
				if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					selected = format;
					matched = true;
				}
			}

			if (!matched) {
				logger::error("No surface format matched the requested parameters!");
			}

			// swapchain creation
			SwapchainBuilder builder {VK_PRESENT_MODE_FIFO_KHR, selected, extent, images, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, transform};
			builder.addSyncedQueue(graphics_ref);
			builder.addSyncedQueue(transfer_ref);
			builder.addSyncedQueue(presentation_ref);

			swapchain = builder.build(device, surface);

		}

		void createFramebuffers(RenderPass& pass) {

			// create the depth buffer shared by the framebuffers
			{
				ImageInfo image_builder {swapchain.vk_extent.width, swapchain.vk_extent.height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};
				image_builder.preferred(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				image_builder.tiling(VK_IMAGE_TILING_OPTIMAL);

				depth_image = allocator.allocateImage(image_builder);
				depth_view = depth_image.getViewBuilder().build(device, VK_IMAGE_ASPECT_DEPTH_BIT);
			}

			// create framebuffers
			framebuffers = swapchain.getFramebuffers(pass, depth_view);

		}

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
			graphics_ref = device_builder.addQueue(QueueType::GRAPHICS, 1);
			transfer_ref = device_builder.addQueue(QueueType::TRANSFER, 1);
			presentation_ref = device_builder.addQueue(surface, 1);
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

			// create swapchain compatible render pass
			createRenderPass(swapchain);

			// create renderpass compatible framebuffers
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

		void recreateSwapchain() {

			// free old stuff
			swapchain.close();
			// FIXME depth_view.close();
			// FIXME depth_image.close();

			for (Framebuffer& framebuffer : framebuffers) {
				framebuffer.close();
			}

			VkFormat old_format = swapchain.vk_surface_format.format;
			framebuffers.clear();

			createSwapchain();

			// we don't necessarily need to recreate the render pass if the format stayed the same
			if (swapchain.vk_surface_format.format != old_format) {
				logger::info("Swapchain format changed, recreating render pass!");
				createRenderPass(swapchain);
			}

			// create new framebuffers
			createFramebuffers(render_pass);

		}

		Frame& getFrame() {
			Frame& frame = frames[index];
			frame.in_flight_fence.lock();
			return frame;
		}

		void nextFrame() {
			index = (index + 1) % concurrent;
		}

};