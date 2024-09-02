
#include "renderer.hpp"

#include "setup/device.hpp"
#include "util/random.hpp"

/*
 * Frame
 */

Frame::Frame(RenderSystem& system, const CommandPool& pool, const Device& device, const ImageSampler& atlas_sampler)
: sequence(INITIAL), buffer(pool.allocate()), immediate_2d(system, 1024), immediate_3d(system, 1024), available_semaphore(device.semaphore()), finished_semaphore(device.semaphore()), flight_fence(device.fence(true)) {

	immediate_2d.setDebugName(device, "Immediate 2D");
	immediate_3d.setDebugName(device, "Immediate 3D");

	set_0 = system.descriptor_pool.allocate(system.geometry_descriptor_layout);
	set_0.sampler(0, system.assets.state->array_sampler);

	set_1 = system.descriptor_pool.allocate(system.geometry_descriptor_layout);
	set_1.sampler(0, atlas_sampler);

	set_2 = system.descriptor_pool.allocate(system.ssao_descriptor_layout);
	set_2.buffer(0, system.ssao_uniform_buffer, sizeof(AmbientOcclusionUniform));
	set_2.sampler(1, system.ssao_noise_sampler);
	set_2.sampler(2, system.attachment_normal.sampler);
	set_2.sampler(3, system.attachment_position.sampler);

	set_3 = system.descriptor_pool.allocate(system.lighting_descriptor_layout);
	set_3.sampler(0, system.attachment_normal.sampler);
	set_3.sampler(1, system.attachment_position.sampler);
	set_3.sampler(2, system.attachment_albedo.sampler);
	set_3.sampler(3, system.attachment_ambience.sampler);

	timestamp_query = QueryPool(system.device, VK_QUERY_TYPE_TIMESTAMP, 2);
}

Frame::~Frame() {
	buffer.close();
	immediate_2d.close();
	immediate_3d.close();
	available_semaphore.close();
	finished_semaphore.close();
	flight_fence.close();
	timestamp_query.close();
}

void Frame::wait() {
	flight_fence.lock();
}

void Frame::execute() {
	queue.execute();

	if (sequence == FIRST) sequence = SUBSEQUENT;
	if (sequence == INITIAL) sequence = FIRST;
}

bool Frame::first() const {
	return sequence != SUBSEQUENT;
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

	{ // geometry pass

		RenderPassBuilder builder;

		Attachment::Ref depth = builder.addAttachment(attachment_depth)
			.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
			.end(ColorOp::STORE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
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

		builder.addDependency(VK_DEPENDENCY_BY_REGION_BIT) // Color Output
			.input(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.output(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_MEMORY_READ_BIT)
			.next();

		builder.addSubpass()
			.addOutput(albedo, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.addOutput(normal, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.addOutput(position, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.addDepth(depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.next();

		terrain_pass = builder.build(device, "Terrain", RenderPass::GREEN);

	}

	{ // SSAO pass

		RenderPassBuilder builder;

		Attachment::Ref depth = builder.addAttachment(attachment_depth)
			.begin(ColorOp::LOAD, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.end(ColorOp::STORE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.next();

		Attachment::Ref normal = builder.addAttachment(attachment_normal)
			.begin(ColorOp::LOAD, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.end(ColorOp::STORE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.next();

		Attachment::Ref ambience = builder.addAttachment(attachment_ambience)
			.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
			.end(ColorOp::STORE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.next();

		builder.addDependency() // G-Buffer/Color 0->Write
			.input(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0)
			.output(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.next();

		builder.addDependency(VK_DEPENDENCY_BY_REGION_BIT) // Color Output
			.input(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.output(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_MEMORY_READ_BIT)
			.next();

		builder.addSubpass()
			.addInput(normal, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.addOutput(ambience, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.addDepth(depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.next();

		ssao_pass = builder.build(device, "SSAO", RenderPass::ORANGE);

	}

	{ // lighting pass

		RenderPassBuilder builder;

		Attachment::Ref color = builder.addAttachment(attachment_color)
			.begin(ColorOp::CLEAR, VK_IMAGE_LAYOUT_UNDEFINED)
			.end(ColorOp::STORE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			.next();

		Attachment::Ref depth = builder.addAttachment(attachment_depth)
			.begin(ColorOp::LOAD, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.end(ColorOp::IGNORE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.next();

		Attachment::Ref albedo = builder.addAttachment(attachment_albedo)
			.begin(ColorOp::LOAD, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.end(ColorOp::IGNORE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.next();

		Attachment::Ref normal = builder.addAttachment(attachment_normal)
			.begin(ColorOp::LOAD, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.end(ColorOp::IGNORE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.next();

		builder.addDependency() // G-Buffer/Color 0->Write
			.input(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0)
			.output(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.next();

		builder.addDependency(VK_DEPENDENCY_BY_REGION_BIT)
			.input(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.output(1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.next();

		builder.addDependency(VK_DEPENDENCY_BY_REGION_BIT) // Color Output
			.input(1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			.output(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_MEMORY_READ_BIT)
			.next();

		builder.addSubpass()
			.addInput(normal, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.addInput(albedo, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			.addOutput(color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.addDepth(depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.next();

		builder.addSubpass()
			.addOutput(color, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			.addDepth(depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			.next();

		lighting_pass = builder.build(device, "Compose", RenderPass::BLUE);

	}

}

// TODO this also would be cool to automate base on render pass attachments
void RenderSystem::createFramebuffers() {

	VkExtent2D extent = swapchain.vk_extent;

	attachment_depth.allocate(device, extent, allocator);
	attachment_albedo.allocate(device, extent, allocator);
	attachment_normal.allocate(device, extent, allocator);
	attachment_position.allocate(device, extent, allocator);
	attachment_ambience.allocate(device, extent, allocator);

	// create the deferred framebuffer
	{
		FramebufferBuilder builder {terrain_pass, extent};
		builder.addAttachment(attachment_depth);
		builder.addAttachment(attachment_albedo);
		builder.addAttachment(attachment_normal);
		builder.addAttachment(attachment_position);

		terrain_framebuffer = builder.build(device);
	}

	// create the deferred framebuffer
	{
		FramebufferBuilder builder {ssao_pass, extent};
		builder.addAttachment(attachment_depth);
		builder.addAttachment(attachment_normal);
		builder.addAttachment(attachment_ambience);

		ssao_framebuffer = builder.build(device);
	}

	const std::vector<ImageView>& views = swapchain.getImageViews();

	// create the main screen framebuffers
	framebuffers.clear();
	framebuffers.reserve(views.size());

	for (const ImageView& view : views) {
		FramebufferBuilder builder {lighting_pass, extent};
		builder.addAttachment(view);
		builder.addAttachment(attachment_depth);
		builder.addAttachment(attachment_albedo);
		builder.addAttachment(attachment_normal);

		framebuffers.push_back(builder.build(device, framebuffers.size()));
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
		terrain_pass.close();
		createRenderPass();
	}

	// create new framebuffers
	createFramebuffers();

	pipeline_2d_tint.close();
	pipeline_3d_terrain.close();
	pipeline_3d_tint.close();

	createPipelines();
	createFrames();

	instance.enterValidationCheckpoint("Render System Swapchain Reload");

}

void RenderSystem::createPipelines() {

	VkExtent2D extent = swapchain.vk_extent;

	pipeline_3d_terrain = GraphicsPipelineBuilder::of(device)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withCulling(true)
		.withRenderPass(terrain_pass, 0)
		.withShaders(assets.state->vert_terrain, assets.state->frag_terrain)
		.withDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true, true)
		.withBindingLayout(binding_terrain)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(geometry_descriptor_layout)
		.withDebugName("Terrain")
		.build();

	pipeline_3d_tint = GraphicsPipelineBuilder::of(device)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withCulling(true)
		.withRenderPass(lighting_pass, 1)
		.withShaders(assets.state->vert_3d, assets.state->frag_tint)
		.withDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true, true)
		.withBlendMode(BlendMode::ENABLED)
		.withBlendAlphaFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBindingLayout(binding_3d)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(geometry_descriptor_layout)
		.withDebugName("Simple 3D")
		.build();

	pipeline_2d_tint = GraphicsPipelineBuilder::of(device)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withRenderPass(lighting_pass, 1)
		.withShaders(assets.state->vert_2d, assets.state->frag_tint)
		.withBlendMode(BlendMode::ENABLED)
		.withBlendAlphaFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBindingLayout(binding_2d)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(geometry_descriptor_layout)
		.withDebugName("Simple 2D")
		.build();

	pipeline_ssao = GraphicsPipelineBuilder::of(device)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withRenderPass(ssao_pass, 0)
		.withDepthTest(VK_COMPARE_OP_GREATER, true, false)
		.withShaders(assets.state->vert_blit, assets.state->frag_ssao)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(ssao_descriptor_layout)
		.withDebugName("SSAO")
		.build();

	pipeline_compose = GraphicsPipelineBuilder::of(device)
		.withViewport(0, 0, extent.width, extent.height)
		.withScissors(0, 0, extent.width, extent.height)
		.withRenderPass(lighting_pass, 0)
		.withDepthTest(VK_COMPARE_OP_GREATER, true, false)
		.withShaders(assets.state->vert_blit, assets.state->frag_compose)
		.withPushConstantLayout(constant_layout)
		.withDescriptorSetLayout(lighting_descriptor_layout)
		.withDebugName("Lighting")
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
		frames.emplace_back(*this, graphics_pool, device, assets.getAtlasSampler());
	}
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

	// function for naming things, from the debug utils extension
	#if !defined(NDEBUG)
	loadDeviceFunction(device.vk_device, "vkSetDebugUtilsObjectNameEXT");
	loadDeviceFunction(device.vk_device, "vkCmdBeginDebugUtilsLabelEXT");
	loadDeviceFunction(device.vk_device, "vkCmdEndDebugUtilsLabelEXT");
	loadDeviceFunction(device.vk_device, "vkCmdInsertDebugUtilsLabelEXT");
	#endif

	// create command pools
	graphics_pool = CommandPool::build(device, graphics_ref, false);
	transient_pool = CommandPool::build(device, transfer_ref, true);

	// get the VMA allocator ready
	allocator = Allocator {device, instance};

	// this is NEEDED so that `frames` vector does not relocate and deconstruct stored frames during insertion
	frames.reserve(concurrent);

	// binding layout used by world renderer
	binding_terrain = BindingLayoutBuilder::begin()
		.attribute(0, VK_FORMAT_R32G32B32_SFLOAT) // xyz
		.attribute(1, VK_FORMAT_R32_UINT)         // sprite index
		.attribute(2, VK_FORMAT_R32_UINT)         // packed UV
		.attribute(3, VK_FORMAT_R8G8B8_UNORM)     // rgb
		.attribute(4, VK_FORMAT_R8_UINT)          // normal
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

	constant_layout = PushConstantLayoutBuilder::begin()
		.add(Kind::VERTEX | Kind::FRAGMENT, 64 + 64, &push_constant)
		.done();

	attachment_depth = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_D32_SFLOAT)
		.setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		.setAspect(VK_IMAGE_ASPECT_DEPTH_BIT)
		.setDepthClearValue(1.0f)
		.setDebugName("Depth")
		.build();

	attachment_albedo = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_R8G8B8A8_UNORM)
		.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setColorClearValue(0, 0, 0, 0)
		.setDebugName("Albedo")
		.build();

	attachment_normal = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_R16G16B16A16_SFLOAT)
		.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setMode(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT)
		.setColorClearValue(0, 0, 0, 0)
		.setDebugName("Normal")
		.build();

	attachment_position = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_R32G32B32A32_SFLOAT)
		.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setMode(VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT)
		.setColorClearValue(0, 0, 0, 0)
		.setDebugName("Position")
		.build();

	attachment_ambience = AttachmentImageBuilder::begin()
		.setFormat(VK_FORMAT_R8_UNORM)
		.setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
		.setAspect(VK_IMAGE_ASPECT_COLOR_BIT)
		.setColorClearValue(0, 0, 0, 0)
		.setDebugName("Ambience")
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

		glm::vec3 sample;
		sample.x = random.uniformFloat(-1, 1);
		sample.y = random.uniformFloat(-1, 1);
		sample.z = 0;

		ssao_noise.emplace_back(sample, 0.0f);
	}

	for (int i = 0; i < 64; ++ i) {

		glm::vec3 sample;
		sample.x = random.uniformFloat(-1, 1);
		sample.y = random.uniformFloat(-1, 1);
		sample.z = random.uniformFloat(0, 1);

		sample = glm::normalize(sample);
		sample *= random.uniformFloat(0, 1);

		float frac = i / 64.0f;
		float scale = std::lerp(0.1f, 1.0f, frac * frac);
		sample *= scale;

		ssao_kernel.emplace_back(sample, 0.0f);

	}

	memcpy(ssao_config.samples, ssao_kernel.data(), 64 * sizeof(glm::vec4));
	ssao_config.noise_scale = glm::vec2 {1000.0/4.0, 700.0/4.0};

	ssao_noise_image = ImageData::view(ssao_noise.data(), 4, 4, sizeof(glm::vec4)).upload(allocator, transient_buffers, transient_recorder, VK_FORMAT_R32G32B32A32_SFLOAT, false);
	ssao_noise_view = ssao_noise_image.getViewBuilder().build(device, VK_IMAGE_ASPECT_COLOR_BIT);
	ssao_noise_sampler = ssao_noise_view.getSamplerBuilder().setMode(VK_SAMPLER_ADDRESS_MODE_REPEAT).setFilter(VK_FILTER_NEAREST).build(device);

	ssao_noise_image.setDebugName(device, "SSAO Noise");
	ssao_noise_view.setDebugName(device, "SSAO Noise");
	ssao_noise_sampler.setDebugName(device, "SSAO Noise");

	BufferInfo ssao_buffer_builder {sizeof(AmbientOcclusionUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};
	ssao_buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	ssao_buffer_builder.preferred(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	ssao_buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

	ssao_uniform_buffer = allocator.allocateBuffer(ssao_buffer_builder);
	ssao_uniform_buffer.setDebugName(device, "SSAO Uniform");

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
		.descriptor(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
		.descriptor(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.done(device);

	lighting_descriptor_layout = DescriptorSetLayoutBuilder::begin()
		.descriptor(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
		.descriptor(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.descriptor(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
		.descriptor(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.done(device);

	geometry_descriptor_layout = DescriptorSetLayoutBuilder::begin()
		.descriptor(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.done(device);

	descriptor_pool = DescriptorPoolBuilder::begin()
		.addDynamic(ssao_descriptor_layout, 1)
		.addDynamic(lighting_descriptor_layout, 1)
		.addDynamic(geometry_descriptor_layout, 2)
		.done(device, concurrent);

	instance.enterValidationCheckpoint("Render System Phase 1 Initialization");

	// Phase 2
	// this step needs to be more or less repeated every time the window size changes

	createSwapchain();
	createRenderPass();
	createFramebuffers();

	instance.enterValidationCheckpoint("Render System Phase 2 Initialization");

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

	instance.enterValidationCheckpoint("Render System Phase 3 Initialization");
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
		pipeline_ssao.close();
		pipeline_compose.close();

		createPipelines();
		createFrames();
	}).milliseconds(), "ms");

	instance.enterValidationCheckpoint("Render System Resource Reload");

}

Framebuffer& RenderSystem::acquireScreenFramebuffer() {
	uint32_t image_index;
	if (swapchain.getNextImage(getFrame().available_semaphore, &image_index).mustReplace()) {
		recreateSwapchain();
	}

	return framebuffers[image_index];
}

void RenderSystem::presentScreenFramebuffer(const Framebuffer& framebuffer) {
	auto image_index = framebuffer.presentation_index;
	auto semaphore = getFrame().finished_semaphore;

	//TODO rn it sometimes causes the queue submission to fail
	//presenter.enqueue([this, image_index, semaphore] () {
		if (swapchain.present(presentation_queue, semaphore, image_index).mustReplace()) {
			recreateSwapchain();
		}
	//});
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
	attachment_normal.close(device);
	attachment_position.close(device);
	attachment_albedo.close(device);
	attachment_ambience.close(device);

	terrain_framebuffer.close();
	ssao_framebuffer.close();

	for (Framebuffer& framebuffer : framebuffers) {
		framebuffer.close();
	}

	closeFrames();

	pipeline_2d_tint.close();
	pipeline_3d_terrain.close();
	pipeline_3d_tint.close();
	pipeline_ssao.close();
	pipeline_compose.close();

	terrain_pass.close();
	lighting_pass.close();
	ssao_pass.close();

	geometry_descriptor_layout.close();
	ssao_descriptor_layout.close();
	lighting_descriptor_layout.close();

	ssao_noise_sampler.close(device);
	ssao_noise_view.close(device);
	ssao_noise_image.close();
	ssao_uniform_buffer.close();

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