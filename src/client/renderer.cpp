
#include "renderer.hpp"

#include "setup/device.hpp"

/*
 * Frame
 */

Frame::Frame(RenderSystem& system, const CommandPool& pool, const Device& device, DescriptorSet descriptor1, DescriptorSet descriptor2, const ImageSampler& atlas_sampler, const ImageSampler& albedo_sampler, const ImageSampler& normal_sampler, const ImageSampler& position_sampler)
: buffer(pool.allocate()), immediate_2d(system, 1024), immediate_3d(system, 1024), available_semaphore(device.semaphore()), finished_semaphore(device.semaphore()), flight_fence(device.fence(true)) {

//	// leaving this here as it may prove useful later
//	BufferInfo buffer_builder {sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};
//	buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//	buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
//
//	ubo = system.allocator.allocateBuffer(buffer_builder);
//	map = ubo.access().map();

	set1 = descriptor1;
	set1.sampler(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, atlas_sampler);

	set2 = descriptor2;
	set2.sampler(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, albedo_sampler);
	set2.sampler(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, normal_sampler);
	set2.sampler(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, position_sampler);
}

Frame::~Frame() {
	buffer.close();
	immediate_2d.close();
	immediate_3d.close();
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
		logger::debug("Executed ", count, " deferred frame tasks");
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
		throw Exception {"No surface format matched the requested parameters!"};
	}

	// swapchain creation
	SwapchainBuilder builder {VK_PRESENT_MODE_IMMEDIATE_KHR, selected, extent, images, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, transform};
	builder.addSyncedQueue(graphics_queue);
	builder.addSyncedQueue(transfer_queue);
	builder.addSyncedQueue(presentation_queue);

	swapchain = builder.build(device, surface);

	attachment_color = AttachmentImageBuilder::begin()
		.setFormat(selected.format)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setColorClearValue(0.0f, 0.0f, 0.0f, 1.0f)
		.build();

}

void RenderSystem::createRenderPass(Swapchain& surface) {

	RenderPassBuilder builder;

	Attachment::Ref color = builder.addAttachment(attachment_color)
		.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
		.end(ColorOp::STORE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		.next();

	Attachment::Ref albedo = builder.addAttachment(attachment_albedo)
		.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
		.end(ColorOp::IGNORE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.next();

	Attachment::Ref normal = builder.addAttachment(attachment_normal)
		.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
		.end(ColorOp::IGNORE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.next();

	Attachment::Ref position = builder.addAttachment(attachment_position)
		.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
		.end(ColorOp::IGNORE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.next();

	Attachment::Ref depth = builder.addAttachment(attachment_depth)
		.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
		.end(ColorOp::IGNORE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.next();

	builder.addDependency() // G-Buffer/Color 0->Write
		.input(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0)
		.output(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.next();

	builder.addDependency() // Depth 0->Write
		.input(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0)
		.output(0, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
		.next();

	builder.addDependency(VK_DEPENDENCY_BY_REGION_BIT) // G-Buffer Write->Read
		.input(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.output(1, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_INPUT_ATTACHMENT_READ_BIT)
		.next();

	builder.addDependency(VK_DEPENDENCY_BY_REGION_BIT) // G-Buffer Write->Write
		.input(1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.output(2, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.next();

	builder.addDependency(VK_DEPENDENCY_BY_REGION_BIT) // Color Output
		.input(2, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.output(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_MEMORY_READ_BIT)
		.next();

	builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
		.addOutput(albedo, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.addOutput(normal, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.addOutput(position, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.addDepth(depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.next();

	builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
		.addInput(albedo, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.addInput(normal, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.addInput(position, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.addOutput(color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.next();

	builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
		.addOutput(color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.addDepth(depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.next();

	render_pass = builder.build(device);

}

void RenderSystem::createFramebuffers(RenderPass& pass) {

	VkExtent2D extent = swapchain.vk_extent;

	attachment_depth.allocate(device, extent, allocator);
	attachment_albedo.allocate(device, extent, allocator);
	attachment_normal.allocate(device, extent, allocator);
	attachment_position.allocate(device, extent, allocator);

	const std::vector<Image>& images = swapchain.getImages();

	// create framebuffers
	framebuffers.clear();
	framebuffers.reserve(images.size());

	for (const Image& image : images) {
		FramebufferBuilder builder {pass, extent};
		builder.addAttachment(image.getViewBuilder().build(device, VK_IMAGE_ASPECT_COLOR_BIT), true);
		builder.addAttachment(attachment_albedo.view);
		builder.addAttachment(attachment_normal.view);
		builder.addAttachment(attachment_position.view);
		builder.addAttachment(attachment_depth.view);

		framebuffers.push_back(builder.build(device, framebuffers.size()));
	}

}

void RenderSystem::recreateSwapchain() {

	// wait so that we can delete whatever we like
	wait();

	// free old stuff
	swapchain.close();

	attachment_depth.close(device);
	attachment_normal.close(device);
	attachment_albedo.close(device);
	attachment_position.close(device);

	for (Framebuffer& framebuffer : framebuffers) {
		framebuffer.close();
	}

	VkFormat old_format = swapchain.vk_surface_format.format;
	framebuffers.clear();

	createSwapchain();

	// we don't necessarily need to recreate the render pass if the format stayed the same
	if (swapchain.vk_surface_format.format != old_format) {
		logger::info("Swapchain format changed, recreating render passes!");
		render_pass.close();
		createRenderPass(swapchain);
	}

	// create new framebuffers
	createFramebuffers(render_pass);

	pipeline_2d_tint.close();
	pipeline_3d_terrain.close();
	pipeline_3d_tint.close();
	pipeline_2d_compose.close();

	createPipelines();
	createFrames();
}

void RenderSystem::createPipelines() {

	VkExtent2D extent = swapchain.vk_extent;

	pipeline_3d_terrain = GraphicsPipelineBuilder::of(device, 3)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withCulling(true)
		.withRenderPass(render_pass, 0)
		.withShaders(assets.state->vert_terrain, assets.state->frag_terrain)
		.withDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true, true)
		.withBindingLayout(binding_terrain)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(descriptor_layout)
		.build();

	pipeline_2d_compose = GraphicsPipelineBuilder::of(device, 1)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withRenderPass(render_pass, 1)
		.withShaders(assets.state->vert_compose, assets.state->frag_compose)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(composition_layout)
		.build();

	pipeline_3d_tint = GraphicsPipelineBuilder::of(device, 1)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withCulling(true)
		.withRenderPass(render_pass, 2)
		.withShaders(assets.state->vert_3d, assets.state->frag_tint)
		.withDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true, true)
		.withBlendMode(BlendMode::ENABLED)
		.withBlendAlphaFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBindingLayout(binding_3d)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(descriptor_layout)
		.build();

	pipeline_2d_tint = GraphicsPipelineBuilder::of(device, 1)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withRenderPass(render_pass, 2)
		.withShaders(assets.state->vert_2d, assets.state->frag_tint)
		.withBlendMode(BlendMode::ENABLED)
		.withBlendAlphaFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBindingLayout(binding_2d)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(descriptor_layout)
		.build();

}

void RenderSystem::createFrames() {
	if (!frames.empty()) {

		// execute all pending operations to not leak any memory
		for (Frame& frame : frames) {
			frame.execute();
		}

		frames.clear();
		descriptor_pool.reset();
		graphics_pool.reset(true);
		transient_pool.reset(true);
	}

	for (int i = 0; i < concurrent; i ++) {
		DescriptorSet descriptor_1 = descriptor_pool.allocate(descriptor_layout);
		DescriptorSet descriptor_2 = descriptor_pool.allocate(composition_layout);

		frames.emplace_back(*this, graphics_pool, device, descriptor_1, descriptor_2, assets.getAtlasSampler(), attachment_albedo.sampler, attachment_normal.sampler, attachment_position.sampler);
	}
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

	composition_layout = DescriptorSetLayoutBuilder::begin()
		.descriptor(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
		.descriptor(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
		.descriptor(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
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
		.add(32, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		.add(32, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		.add(32, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
		.done(device, 32);

	constant_layout = PushConstantLayoutBuilder::begin()
		.add(Kind::VERTEX, 64, &mvp_vertex_constant)
		.add(Kind::FRAGMENT, 16, &sun_vertex_constant)
		.done();

	attachment_depth = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_D32_SFLOAT)
		.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		.setAspect(VK_IMAGE_ASPECT_DEPTH_BIT)
		.setDepthClearValue(1.0f)
		.build();

	attachment_albedo = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_R8G8B8A8_UNORM)
		.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setColorClearValue(1.0f, 0.0f, 0.0f, 0.0f)
		.build();

	attachment_normal = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
		.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setColorClearValue(0.0f, 0.0f, 0.0f, 0.0f)
		.build();

	attachment_position = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
		.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setColorClearValue(0.0f, 0.0f, 0.0f, 0.0f)
		.build();

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
	createFrames();
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
		createFrames();
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
	// On some std::vector implementations (Windows) the elements is only treated as inserted after it's constructor
	// is done (container size is incremented after the element is constructed), this makes referencing it by index
	// from that constructor throw in debug mode. This hack works around that and references it directly, note that
	// this is safe as 'frames' will never be re-allocated as that is something we ensure separately using 'reserve()'
	// before we insert anything so we can, in fact, be sure that the element is being constructed "in-place".
	frames.data()[index].queue.enqueue(task);
}

void RenderSystem::wait() {
	device.wait();
}