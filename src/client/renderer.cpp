
#include "renderer.hpp"

#include "setup/device.hpp"
#include "util/random.hpp"

/*
 * Frame
 */

Frame::Frame(RenderSystem& system, const CommandPool& pool, const Device& device, DescriptorSet descriptor_1, const ImageSampler& atlas_sampler)
: buffer(pool.allocate()), immediate_2d(system, 1024), immediate_3d(system, 1024), available_semaphore(device.semaphore()), finished_semaphore(device.semaphore()), flight_fence(device.fence(true)) {

	set_1 = system.descriptor_pool.allocate(system.descriptor_layout);
	set_1.sampler(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, atlas_sampler);

	set_2 = system.descriptor_pool.allocate(system.ssao_descriptor_layout);
	set_2.buffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, system.ssao_uniform_buffer, sizeof(AmbientOcclusionUniform));
	set_2.sampler(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, system.ssao_noise_sampler);
	set_2.sampler(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, system.attachment_normal.sampler);
	set_2.sampler(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, system.attachment_position.sampler);
	set_2.sampler(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, system.attachment_albedo.sampler);

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
	queue.execute();
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

void RenderSystem::createRenderPass() {

	RenderPassBuilder builder;

	Attachment::Ref color = builder.addAttachment(attachment_color)
		.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
		.end(ColorOp::STORE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		.next();

	Attachment::Ref depth = builder.addAttachment(attachment_depth)
		.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
		.end(ColorOp::IGNORE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.next();

	Attachment::Ref albedo = builder.addAttachment(attachment_albedo)
		.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
		.end(ColorOp::STORE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.next();

	Attachment::Ref normal = builder.addAttachment(attachment_normal)
		.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
		.end(ColorOp::STORE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.next();

	Attachment::Ref position = builder.addAttachment(attachment_position)
		.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
		.end(ColorOp::STORE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.next();

	builder.addDependency() // G-Buffer/Color 0->Write
		.input(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0)
		.output(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.next();

	builder.addDependency() // Depth 0->Write
		.input(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0)
		.output(0, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
		.next();

	builder.addDependency(VK_DEPENDENCY_BY_REGION_BIT)
		.input(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.output(1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.next();

	builder.addDependency(VK_DEPENDENCY_BY_REGION_BIT) // Color Output
		.input(1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.output(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_MEMORY_READ_BIT)
		.next();

	builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
		.addOutput(color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.addOutput(albedo, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.addOutput(normal, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.addOutput(position, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.addDepth(depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.next();

	builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
		.addOutput(color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.addDepth(depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.next();

	render_pass = builder.build(device);

	RenderPassBuilder ssao_builder;

	Attachment::Ref ambience = ssao_builder.addAttachment(attachment_ambience)
		.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
		.end(ColorOp::STORE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.next();

	ssao_builder.addDependency() // G-Buffer/Color 0->Write
		.input(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0)
		.output(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.next();

	ssao_builder.addDependency(VK_DEPENDENCY_BY_REGION_BIT) // Color Output
		.input(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.output(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_MEMORY_READ_BIT)
		.next();

	ssao_builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
		.addOutput(ambience, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.next();

	ssao_render_pass = ssao_builder.build(device);

}

void RenderSystem::createFramebuffers() {

	VkExtent2D extent = swapchain.vk_extent;

	attachment_depth.allocate(device, extent, allocator);
	attachment_albedo.allocate(device, extent, allocator);
	attachment_normal.allocate(device, extent, allocator);
	attachment_position.allocate(device, extent, allocator);
	attachment_ambience.allocate(device, extent, allocator);

	const std::vector<Image>& images = swapchain.getImages();

	// create the main screen framebuffers
	framebuffers.clear();
	framebuffers.reserve(images.size());

	for (const Image& image : images) {
		FramebufferBuilder builder {render_pass, extent};
		builder.addAttachment(image.getViewBuilder().build(device, VK_IMAGE_ASPECT_COLOR_BIT), true);
		builder.addAttachment(attachment_depth);
		builder.addAttachment(attachment_albedo);
		builder.addAttachment(attachment_normal);
		builder.addAttachment(attachment_position);

		framebuffers.push_back(builder.build(device, framebuffers.size()));
	}

	// create the SSAO framebuffer
	{
		FramebufferBuilder builder {ssao_render_pass, extent};
		builder.addAttachment(attachment_ambience);

		ssao_framebuffer = builder.build(device);
	}
}

void RenderSystem::recreateSwapchain() {

	// wait so that we can delete whatever we like
	wait();

	// free old stuff
	swapchain.close();
	attachment_depth.close(device);

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
		createRenderPass();
	}

	// create new framebuffers
	createFramebuffers();

	pipeline_2d_tint.close();
	pipeline_3d_terrain.close();
	pipeline_3d_tint.close();

	createPipelines();
	createFrames();
}

void RenderSystem::createPipelines() {

	VkExtent2D extent = swapchain.vk_extent;

	pipeline_3d_terrain = GraphicsPipelineBuilder::of(device)
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

	pipeline_3d_tint = GraphicsPipelineBuilder::of(device)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withCulling(true)
		.withRenderPass(render_pass, 1)
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
		.withRenderPass(render_pass, 1)
		.withShaders(assets.state->vert_2d, assets.state->frag_tint)
		.withBlendMode(BlendMode::ENABLED)
		.withBlendAlphaFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBindingLayout(binding_2d)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(descriptor_layout)
		.build();

	ssao_pipeline = GraphicsPipelineBuilder::of(device)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withRenderPass(ssao_render_pass, 0)
		.withShaders(assets.state->vert_blit, assets.state->frag_ssao)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(ssao_descriptor_layout)
		.build();

}

void RenderSystem::closeFrames() {
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
}

void RenderSystem::createFrames() {
	closeFrames();

	for (int i = 0; i < concurrent; i ++) {
		DescriptorSet descriptor_1 = descriptor_pool.allocate(descriptor_layout);
		frames.emplace_back(*this, graphics_pool, device, descriptor_1, assets.getAtlasSampler());
	}
}

float learn_opengl_lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

RenderSystem::RenderSystem(Window& window, int concurrent)
: window(window), concurrent(concurrent), index(0) {

	// Phase 1
	// this step is only ever executed once

	// instance configuration
	InstanceBuilder builder;
	builder.addApplicationInfo("My Funny Vulkan Application");
	builder.addInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME).orFail();

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
	FEATURE_ENABLE_OR_FAIL(device_builder, vk_features.features.fillModeNonSolid);
	FEATURE_ENABLE_OR_FAIL(device_builder, vk_features.features.wideLines);

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

	// TODO
	descriptor_pool = DescriptorPoolBuilder::begin()
		.add(32, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		.add(32, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		.add(32, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
		.done(device, 32);

	constant_layout = PushConstantLayoutBuilder::begin()
		.add(Kind::VERTEX | Kind::FRAGMENT, 64 + 64, &push_constant)
		.done();

	attachment_depth = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_D32_SFLOAT)
		.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		.setAspect(VK_IMAGE_ASPECT_DEPTH_BIT)
		.setDepthClearValue(1.0f)
		.build();

	attachment_albedo = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_R8G8B8A8_UNORM)
		.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setColorClearValue(0, 0, 0, 0)
		.build();

	attachment_normal = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
		.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setColorClearValue(0, 0, 0, 0)
		.build();

	attachment_position = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
		.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setColorClearValue(0, 0, 0, 0)
		.build();

	attachment_ambience = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_R8_UNORM)
		.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setColorClearValue(0, 0, 0, 0)
		.build();

	Random random {42};
	TaskQueue transient_buffers;
	CommandBuffer transient_commands = transient_pool.allocate();
	CommandRecorder transient_recorder = transient_commands.record(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	Fence fence = device.fence();

	std::vector<glm::vec4> ssao_noise;
	std::vector<glm::vec4> ssao_kernel;
	AmbientOcclusionUniform ssao_config;

	for (unsigned int i = 0; i < 16; i ++) {

		glm::vec4 sample;
		sample.x = random.uniformFloat(-1, 1);
		sample.y = random.uniformFloat(-1, 1);
		sample.z = 0;
		sample.w = 0;

		ssao_noise.push_back(sample);
	}

	for (int i = 0; i < 64; ++ i) {

		glm::vec3 sample;
		sample.x = random.uniformFloat(-1, 1);
		sample.y = random.uniformFloat(-1, 1);
		sample.z = random.uniformFloat(0, 1);

		sample = glm::normalize(sample);
		sample *= random.uniformFloat(0, 1);

		float frac = i / 64.0f;
		float scale = learn_opengl_lerp(0.1f, 1.0f, frac * frac);
		sample *= scale;

		ssao_kernel.emplace_back(sample, 0.0f);

	}

	memcpy(ssao_config.samples, ssao_kernel.data(), 64 * sizeof(glm::vec3));

	this->ssao_noise_image = ImageData::view(ssao_noise.data(), 4, 4, sizeof(glm::vec4)).upload(allocator, transient_buffers, transient_recorder, VK_FORMAT_R32G32B32A32_SFLOAT);
	this->ssao_noise_view = ssao_noise_image.getViewBuilder().build(device, VK_IMAGE_ASPECT_COLOR_BIT);
	this->ssao_noise_sampler = ssao_noise_view.getSamplerBuilder().setMode(VK_SAMPLER_ADDRESS_MODE_REPEAT).setFilter(VK_FILTER_NEAREST).build(device);

	BufferInfo ssao_buffer_builder {sizeof(AmbientOcclusionUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};
	ssao_buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	ssao_buffer_builder.preferred(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	ssao_buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

	ssao_uniform_buffer = allocator.allocateBuffer(ssao_buffer_builder);

	MemoryMap ssao_map = ssao_uniform_buffer.access().map();
	ssao_map.write(&ssao_config, sizeof(AmbientOcclusionUniform));
	ssao_map.unmap();

	transient_recorder.done();
	transient_commands.submit().unlocks(fence).done(transfer_queue);
	fence.wait();
	fence.close();
	transient_commands.close();
	transient_buffers.execute();

	ssao_descriptor_layout = DescriptorSetLayoutBuilder::begin()
		.descriptor(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.descriptor(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.descriptor(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.descriptor(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.descriptor(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.done(device);

	// Phase 2
	// this step needs to be more or less repeated every time the window size changes

	createSwapchain();
	createRenderPass();
	createFramebuffers();

	// Phase 3
	// this step will need to be repeated each time the resources are reloaded

	logger::info("Resource reload took: ", Timer::of([&] {
		TaskQueue queue;
		Fence fence = device.fence();
		CommandBuffer buffer = transient_pool.allocate();
		assets.reload(*this, queue, buffer);
		buffer.submit().unlocks(fence).done(transfer_queue);

		fence.wait();
		fence.close();
		buffer.close();
		queue.execute();
	}).milliseconds(), "ms");

	createPipelines();
	createFrames();
}

void RenderSystem::reloadAssets() {
	wait();

	logger::info("Resource reload took: ", Timer::of([&] {
		TaskQueue queue;
		Fence fence = device.fence();
		CommandBuffer buffer = transient_pool.allocate();
		assets.reload(*this, queue, buffer);
		buffer.submit().unlocks(fence).done(transfer_queue);

		fence.wait();
		fence.close();
		buffer.close();
		queue.execute();

		pipeline_2d_tint.close();
		pipeline_3d_terrain.close();
		pipeline_3d_tint.close();

		createPipelines();
		createFrames();
	}).milliseconds(), "ms");
}

Framebuffer& RenderSystem::acquireScreenFramebuffer() {
	uint32_t image_index;
	if (swapchain.getNextImage(getFrame().available_semaphore, &image_index).mustReplace()) {
		recreateSwapchain();
	}

	return framebuffers[image_index];
}

void RenderSystem::presentScreenFramebuffer(Framebuffer& framebuffer) {
	if (swapchain.present(presentation_queue, getFrame().finished_semaphore, framebuffer.presentation_index).mustReplace()) {
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

void RenderSystem::close() {
	wait();

	swapchain.close();
	attachment_depth.close(device);

	for (Framebuffer& framebuffer : framebuffers) {
		framebuffer.close();
	}

	closeFrames();

	pipeline_2d_tint.close();
	pipeline_3d_terrain.close();
	pipeline_3d_tint.close();

	render_pass.close();
	descriptor_layout.close();

	assets.close();
	transient_pool.close();
	graphics_pool.close();
	descriptor_pool.close();
	surface.close();
	allocator.close();

	// Goodbye vulkan!
	device.close();
	instance.close();

}