
#include "renderer.hpp"

#include "setup/device.hpp"

/*
 * Frame
 */

Frame::Frame(RenderSystem& system, const CommandPool& pool, const Device& device, DescriptorSet descriptor, const ImageSampler& sampler)
: buffer(pool.allocate()), immediate_2d(system, 1024), immediate_3d(system, 1024), available_semaphore(device.semaphore()), finished_semaphore(device.semaphore()), flight_fence(device.fence(true)) {

//	// leaving this here as it may prove useful later
//	BufferInfo buffer_builder {sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};
//	buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//	buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
//
//	ubo = system.allocator.allocateBuffer(buffer_builder);
//	map = ubo.access().map();

	set = descriptor;
	set.sampler(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sampler);
}

Frame::~Frame() {
	buffer.close();
	available_semaphore.close();
	finished_semaphore.close();
	flight_fence.close();
}

void Frame::wait() {
	flight_fence.lock();
}

void Frame::execute() {
	int count = queue.execute();

	if (count > 0) {
		logger::debug("Executed ", count, " defered frame tasks");
	}
}

/*
 * RenderSystem
 */

DeviceBuilder RenderSystem::pickDevice(Instance& instance, WindowSurface& surface) {
	for (DeviceInfo& device : instance.getDevices()) {
		if (device.getQueueFamily(QueueType::GRAPHICS) && device.getQueueFamily(surface) && device.hasSwapchain(surface)) {
			logger::info("Selected device: ", device.getProperties().deviceName);
			return device.builder();
		}
	}

	throw Exception {"No viable Vulkan device found!"};
}

void RenderSystem::createSwapchain() {

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
	SwapchainBuilder builder {VK_PRESENT_MODE_IMMEDIATE_KHR, selected, extent, images, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, transform};
	builder.addSyncedQueue(graphics_queue);
	builder.addSyncedQueue(transfer_queue);
	builder.addSyncedQueue(presentation_queue);

	swapchain = builder.build(device, surface);

}

void RenderSystem::createRenderPass(Swapchain& surface) {

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

void RenderSystem::createFramebuffers(RenderPass& pass) {

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

void RenderSystem::recreateSwapchain() {

	// wait so that we can delete whatever we like
	wait();

	// free old stuff
	swapchain.close();
	depth_view.close(device);
	depth_image.close(device);

	for (Framebuffer& framebuffer : framebuffers) {
		framebuffer.close();
	}

	VkFormat old_format = swapchain.vk_surface_format.format;
	framebuffers.clear();

	createSwapchain();

	// we don't necessarily need to recreate the render pass if the format stayed the same
	if (swapchain.vk_surface_format.format != old_format) {
		logger::info("Swapchain format changed, recreating render pass!");
		render_pass.close();
		createRenderPass(swapchain);
	}

	// create new framebuffers
	createFramebuffers(render_pass);

	pipeline_2d_tint.close();
	pipeline_3d_terrain.close();
	pipeline_3d_tint.close();

	createPipelines();

}

void RenderSystem::createPipelines() {

	VkExtent2D extent = swapchain.vk_extent;

	pipeline_3d_terrain = GraphicsPipelineBuilder::of(device)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withCulling(true)
		.withRenderPass(render_pass)
		.withShaders(assets.state->vert_terrain, assets.state->frag_terrain)
		.withDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true, true)
		.withBindingLayout(binding_terrain)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(descriptor_layout)
		.build();

	pipeline_3d_tint = GraphicsPipelineBuilder::of(device)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withCulling(true)
		.withRenderPass(render_pass)
		.withShaders(assets.state->vert_3d, assets.state->frag_tint)
		.withDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true, true)
		.withBlendMode(BlendMode::ENABLED)
		.withBlendAlphaFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBindingLayout(binding_3d)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(descriptor_layout)
		.build();

	pipeline_2d_tint = GraphicsPipelineBuilder::of(device)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withRenderPass(render_pass)
		.withShaders(assets.state->vert_2d, assets.state->frag_tint)
		.withBlendMode(BlendMode::ENABLED)
		.withBlendAlphaFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBindingLayout(binding_2d)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(descriptor_layout)
		.build();

}

RenderSystem::RenderSystem(Window& window, int concurrent)
: window(window), concurrent(concurrent), index(0) {

	// Phase 1
	// this step is only ever executed once

	// instance configuration
	InstanceBuilder builder;
	builder.addApplicationInfo("My Funny Vulkan Application");

	#if !defined(NDEBUG)
	logger::debug("Running in debug mode");
	builder.addValidationLayer("VK_LAYER_KHRONOS_validation").orFail();
	builder.addDebugMessenger();
	#endif

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

	// this is NEEDED so that `frames` vector does not relocate and deconstruct stored frames during insertion
	frames.reserve(concurrent);

	// Create this thing
	descriptor_layout = DescriptorSetLayoutBuilder::begin()
		.descriptor(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.done(device);

	// binding layout used by world renderer
	binding_terrain = BindingLayoutBuilder::begin()
		.attribute(0, VK_FORMAT_R32G32B32_SFLOAT)
		.attribute(1, VK_FORMAT_R32G32_SFLOAT)
		.attribute(2, VK_FORMAT_R8G8B8_UNORM)
		.attribute(3, VK_FORMAT_R8_UINT)
		.done();

	// 3D binding layout
	binding_3d = BindingLayoutBuilder::begin()
		.attribute(0, VK_FORMAT_R32G32B32_SFLOAT)
		.attribute(1, VK_FORMAT_R32G32_SFLOAT)
		.attribute(2, VK_FORMAT_R8G8B8A8_UNORM)
		.done();

	// 2D binding layout
	binding_2d = BindingLayoutBuilder::begin()
		.attribute(0, VK_FORMAT_R32G32_SFLOAT)
		.attribute(1, VK_FORMAT_R32G32_SFLOAT)
		.attribute(2, VK_FORMAT_R8G8B8A8_UNORM)
		.done();

	descriptor_pool = DescriptorPoolBuilder::begin()
		.add(concurrent, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		.add(concurrent, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		.done(device, concurrent);

	constant_layout = PushConstantLayoutBuilder::begin()
		.add(Kind::VERTEX, 64, &mvp_vertex_constant)
		.add(Kind::FRAGMENT, 16, &sun_vertex_constant)
		.done();

	// Phase 2
	// this step needs to be more or less repeated every time the window size changes

	createSwapchain();
	createRenderPass(swapchain);
	createFramebuffers(render_pass);

	// Phase 3
	// this step will need to be repeated each time the resources are reloaded

	logger::debug("Resource reload took: ", Timer::of([&] {
		Fence fence = device.fence();
		CommandBuffer buffer = transient_pool.allocate();
		assets.reload(device, allocator, buffer);
		buffer.submit().unlocks(fence).done(transfer_queue);

		fence.wait();
		fence.close();
		buffer.close();
	}).milliseconds(), "ms");

	createPipelines();

	for (int i = 0; i < concurrent; i ++) {
		frames.emplace_back(*this, graphics_pool, device, descriptor_pool.allocate(descriptor_layout), assets.getAtlasSampler());
	}
}

void RenderSystem::reloadAssets() {
	wait();

	logger::debug("Resource reload took: ", Timer::of([&] {
		Fence fence = device.fence();
		CommandBuffer buffer = transient_pool.allocate();
		assets.reload(device, allocator, buffer);
		buffer.submit().unlocks(fence).done(transfer_queue);

		fence.wait();
		fence.close();
		buffer.close();

		pipeline_2d_tint.close();
		pipeline_3d_terrain.close();
		pipeline_3d_tint.close();

		createPipelines();

		descriptor_pool.reset();
		frames.clear();

		for (int i = 0; i < concurrent; i ++) {
			frames.emplace_back(*this, graphics_pool, device, descriptor_pool.allocate(descriptor_layout), assets.getAtlasSampler());
		}
	}).milliseconds(), "ms");

}

Framebuffer& RenderSystem::acquireFramebuffer() {
	uint32_t image_index;
	if (swapchain.getNextImage(getFrame().available_semaphore, &image_index).mustReplace()) {
		recreateSwapchain();
	}

	return framebuffers[image_index];
}

void RenderSystem::presentFramebuffer(Framebuffer& framebuffer) {
	if (swapchain.present(presentation_queue, getFrame().finished_semaphore, framebuffer.index).mustReplace()) {
		recreateSwapchain();
	}
}

Frame& RenderSystem::getFrame() {
	return frames[index];
}

void RenderSystem::nextFrame() {
	index = (index + 1) % concurrent;
}

void RenderSystem::defer(const Task& task) {
	getFrame().queue.enqueue(task);
}

void RenderSystem::wait() {
	device.wait();
}