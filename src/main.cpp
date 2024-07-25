
#include "external.hpp"
#include "context.hpp"
#include "client/camera.hpp"
#include "client/renderer.hpp"
#include "client/vertices.hpp"

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

	Frame(Allocator& allocator, const CommandPool& pool, const Device& device, DescriptorSet descriptor, ImageSampler& sampler)
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

/// Pick a device that has all the features that we need
DeviceBuilder pickDevice(Instance& instance, WindowSurface& surface) {
	for (DeviceInfo& device : instance.getDevices()) {
		if (device.getQueueFamily(QueueType::GRAPHICS) && device.getQueueFamily(surface) && device.hasSwapchain(surface)) {
			logger::info("Selected device: ", device.getProperties().deviceName);
			return device.builder();
		}
	}

	throw std::runtime_error("No viable Vulkan device found!");
}

Swapchain createSwapchain(Device& device, WindowSurface& surface, Window& window, QueueInfo& graphics, QueueInfo& presentation) {

	// swapchain information gathering
	SwapchainInfo info {device, surface};
	auto extent = info.getExtent(window);
	auto images = info.getImageCount();
	auto transform = info.getTransform();

	VkSurfaceFormatKHR selected_format = info.getFormats()[0];

	for (auto& format : info.getFormats()) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			selected_format = format;
		}
	}

	// swapchain creation
	SwapchainBuilder swapchain_builder {VK_PRESENT_MODE_FIFO_KHR, selected_format, extent, images, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, transform};
	swapchain_builder.addSyncedQueue(graphics);
	swapchain_builder.addSyncedQueue(presentation);

	return swapchain_builder.build(device, surface);

}

void recreateSwapchain(Device& device, Allocator& allocator, WindowSurface& surface, Window& window, QueueInfo& graphics, QueueInfo& presentation, RenderPass& pass, std::vector<Framebuffer>& framebuffers, Swapchain& swapchain) {

	device.wait();
	swapchain.close();

	for (Framebuffer& framebuffer : framebuffers) {
		framebuffer.close();
	}

	// FIXME gpu resource leak
	Image depth_image;
	ImageView depth_image_view;

	swapchain = createSwapchain(device, surface, window, graphics, presentation);

	// create the depth buffer
	{
		ImageInfo image_builder {swapchain.vk_extent.width, swapchain.vk_extent.height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};
		image_builder.preferred(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		image_builder.tiling(VK_IMAGE_TILING_OPTIMAL);

		depth_image = allocator.allocateImage(image_builder);
		depth_image_view = depth_image.getViewBuilder().build(device, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	framebuffers = swapchain.getFramebuffers(pass, depth_image_view);

	logger::info("Swapchain recreated!");

}

// for now
#include "world.hpp"
#include "buffer/font.hpp"
#include "client/gui/stack.hpp"
#include "client/gui/screen/test.hpp"
#include "client/gui/screen/group.hpp"

int main() {

	TaskPool pool;

	SoundSystem sound_system;
	SoundBuffer buffer {"assets/sounds/Project_1_mono.ogg"};
	sound_system.add(buffer).loop().play();

	Window window {1000, 700, "Funny Vulkan App"};

	// instance configuration
	InstanceBuilder builder;
	builder.addApplicationInfo("My Funny Vulkan Application");
	builder.addValidationLayer("VK_LAYER_KHRONOS_validation").orFail();
	builder.addDebugMessenger();

	// instance and surface creation, and device selection
	Instance instance = builder.build();
	WindowSurface surface = instance.createSurface(window);
	DeviceBuilder device_builder = pickDevice(instance, surface);

	// device configuration
	QueueInfo graphics_ref = device_builder.addQueue(QueueType::GRAPHICS, 1);
	QueueInfo presentation_ref = device_builder.addQueue(surface, 1);
	device_builder.addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME).orFail();

	// enable some additional features
	device_builder.features.enableFillModeNonSolid().orFail();
	device_builder.features.enableWideLines().orFail();

	// device and queue creation
	Device device = device_builder.create();
	VkQueue graphics = device.get(graphics_ref, 0);
	VkQueue presentation = device.get(presentation_ref, 0);

	// create a compiler and compile the glsl into spirv
	Compiler compiler;
	std::shared_future<ShaderModule> vert_2d = pool.defer([&] { return compiler.compileFile("assets/shaders/vert_2d.glsl", Kind::VERTEX).create(device); }).share();
	std::shared_future<ShaderModule> vert_3d = pool.defer([&] { return compiler.compileFile("assets/shaders/vert_3d.glsl", Kind::VERTEX).create(device); }).share();
	std::shared_future<ShaderModule> frag_mix = pool.defer([&] { return compiler.compileFile("assets/shaders/frag_mix.glsl", Kind::FRAGMENT).create(device); }).share();
	std::shared_future<ShaderModule> frag_tint = pool.defer([&] { return compiler.compileFile("assets/shaders/frag_tint.glsl", Kind::FRAGMENT).create(device); }).share();

	// create VMA based memory allocator
	Allocator allocator {device, instance};

	Atlas atlas = AtlasBuilder::createSimpleAtlas("assets/sprites");

	World world(8888);

	Font font {8};
	font.addCodePage(atlas, "assets/sprites/8x8font.png", 0);

	atlas.getImage().save("atlas.png");

	// swapchain creation
	Swapchain swapchain = createSwapchain(device, surface, window, graphics_ref, presentation_ref);
	auto extent = swapchain.vk_extent;

	// render pass creation
	RenderPassBuilder pass_builder;

	pass_builder.addAttachment(swapchain.vk_surface_format.format, VK_SAMPLE_COUNT_1_BIT)
		.input(ColorOp::CLEAR, StencilOp::IGNORE, VK_IMAGE_LAYOUT_UNDEFINED)
		.output(ColorOp::STORE, StencilOp::IGNORE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		.next();

	pass_builder.addAttachment(VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT)
		.input(ColorOp::CLEAR, StencilOp::IGNORE, VK_IMAGE_LAYOUT_UNDEFINED)
		.output(ColorOp::IGNORE, StencilOp::IGNORE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.next();

	pass_builder.addDependency()
		.input(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0)
		.output(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
		.next();

	pass_builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
		.addColor(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.addDepth(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.next();

	RenderPass pass = pass_builder.build(device);

	Image depth_image;
	ImageView depth_image_view;

	// create the depth buffer for the first time
	{
		ImageInfo image_builder {swapchain.vk_extent.width, swapchain.vk_extent.height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};
		image_builder.preferred(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		image_builder.tiling(VK_IMAGE_TILING_OPTIMAL);

		depth_image = allocator.allocateImage(image_builder);
		depth_image_view = depth_image.getViewBuilder().build(device, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	// create framebuffers
	std::vector<Framebuffer> framebuffers = swapchain.getFramebuffers(pass, depth_image_view);

	// Create this thing
	DescriptorSetLayoutBuilder dsl_builder;
	dsl_builder.descriptor(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
	dsl_builder.descriptor(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
	DescriptorSetLayout layout = dsl_builder.build(device);

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

	GraphicsPipeline pipeline_3d_mix = GraphicsPipelineBuilder::of(device)
		.withDynamics(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR)
		.withRenderPass(pass)
		.withShaders(vert_3d, frag_mix)
		.withDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true, true)
		.withBindingLayout(binding_3d)
		.withDescriptorSetLayout(layout)
		.build("3D Mixed");

	GraphicsPipeline pipeline_3d_tint = GraphicsPipelineBuilder::of(device)
		.withDynamics(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR)
		.withRenderPass(pass)
		.withShaders(vert_3d, frag_tint)
		.withDepthTest(VK_COMPARE_OP_LESS_OR_EQUAL, true, true)
		.withBlendMode(BlendMode::ENABLED)
		.withBlendAlphaFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBindingLayout(binding_3d)
		.withDescriptorSetLayout(layout)
		.build("3D Tinted");

	GraphicsPipeline pipeline_2d_tint = GraphicsPipelineBuilder::of(device)
		.withDynamics(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR)
		.withRenderPass(pass)
		.withShaders(vert_2d, frag_tint)
		.withBlendMode(BlendMode::ENABLED)
		.withBlendAlphaFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBlendColorFunc(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
		.withBindingLayout(binding_2d)
		.withDescriptorSetLayout(layout)
		.build("2D Tinted");

	// create command pools
	CommandPool main_pool = CommandPool::build(device, graphics_ref, false);
	CommandPool transient_pool = CommandPool::build(device, graphics_ref, true);

	// quad texture
	Image image;
	ImageView image_view;
	ImageSampler image_sampler;

	// load file and copy it through a staging buffer into a device local image
	{
		ImageData image_file = atlas.getImage();

		BufferInfo buffer_builder {image_file.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
		buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

		Buffer staging = allocator.allocateBuffer(buffer_builder);

		MemoryMap map = staging.access().map();
		map.write(image_file.data(), image_file.size());
		map.flush();
		map.unmap();

		ImageInfo image_builder {image_file.width(), image_file.height(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT};
		image_builder.preferred(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		image_builder.tiling(VK_IMAGE_TILING_OPTIMAL);

		image = allocator.allocateImage(image_builder);
		image_file.close();

		Fence copy_fence = device.fence();
		CommandBuffer buffer = transient_pool.allocate();

		buffer.record(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
			.transitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED)
			.copyBufferToImage(image, staging, image_file.width(), image_file.height())
			.transitionLayout(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			.done();

		buffer.submit()
			.unlocks(copy_fence)
			.done(graphics);

		copy_fence.wait();
		buffer.close();
		//staging.close();

		image_view = image.getViewBuilder().build(device, VK_IMAGE_ASPECT_COLOR_BIT);
		image_sampler = image_view.getSamplerBuilder().setFilter(VK_FILTER_NEAREST).build(device);
	}

	int concurrent_frames = 1;
	int frame = 0;

	DescriptorPoolBuilder descriptor_pool_builder;
	descriptor_pool_builder.add(concurrent_frames, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	descriptor_pool_builder.add(concurrent_frames, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	DescriptorPool descriptor_pool = descriptor_pool_builder.build(device, concurrent_frames);

	std::vector<Frame> frames;
	std::vector<VkDescriptorSet> sets;

	for (int i = 0; i < concurrent_frames; i ++) {
		frames.emplace_back(allocator, main_pool, device, descriptor_pool.allocate(layout), image_sampler);
	}

	ScreenStack stack;
	ImmediateRenderer renderer {atlas, font};
	Camera camera {window};
	camera.move({0, 5, 0});
	window.setRootInputConsumer(&stack);

	// Open the cluster-fuck screen :D
	stack.open(new TestScreen {});

	BasicBuffer buffer_3d {allocator, 1024};
	BasicBuffer buffer_2d {allocator, 1024};

	while (!window.shouldClose()) {
		window.poll();
		camera.update();
		
		std::list<ChunkBuffer>& buffers = world.getBuffers();

		Frame& ref = frames[frame];
		ref.data.model = glm::identity<glm::mat4>();
		ref.data.proj = glm::perspective(glm::radians(65.0f), swapchain.vk_extent.width / (float) swapchain.vk_extent.height, 0.1f, 1000.0f);
		ref.data.view = camera.getView();

		ref.in_flight_fence.lock();
		ref.map.write(&ref.data, sizeof(UBO));

		// BEGIN THE "AH YES LET'S JUST OPENGL STYLE IT" SECTION
		// * Horribly inefficient
		// * Incompatible with threading and concurrent frames
		world.closeBuffers();
		world.generateAround(camera.getPosition(), 5);
		world.draw(atlas, pool, allocator, camera.getPosition(), 8);

		renderer.prepare(swapchain.vk_extent);
		stack.draw(renderer, window.getInputContext(), camera);
		renderer.write(allocator, buffer_3d, buffer_2d);
		// * Also we now have 3 separate buffer types
		// END THE "AH YES LET'S JUST OPENGL STYLE IT" SECTION

		uint32_t image_index;
		if (swapchain.getNextImage(frames[frame].image_available_semaphore, &image_index).mustReplace()) {
			recreateSwapchain(device, allocator, surface, window, graphics_ref, presentation_ref, pass, framebuffers, swapchain);
			extent = swapchain.vk_extent;
		}

		// record commands
		CommandRecorder commandRecorder = frames[frame].buffer.record()
			.beginRenderPass(pass, framebuffers[image_index], extent, 0.0f, 0.0f, 0.0f, 1.0f)
			.bindPipeline(pipeline_3d_mix)
			.bindDescriptorSet(frames[frame].set)
			.setDynamicViewport(0, 0, extent.width, extent.height)
			.setDynamicScissors(0, 0, extent.width, extent.height);

		for (auto& buffer : buffers) {
			commandRecorder.bindBuffer(buffer.buffer).draw(buffer.size);
		}

		commandRecorder.bindPipeline(pipeline_3d_tint)
			.bindBuffer(buffer_3d.getBuffer())
			.draw(buffer_3d.getCount())
			.bindPipeline(pipeline_2d_tint)
			.bindBuffer(buffer_2d.getBuffer())
			.draw(buffer_2d.getCount())
			.endRenderPass()
			.done();

		frames[frame].buffer.submit()
			.awaits(frames[frame].image_available_semaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			.unlocks(frames[frame].render_finished_semaphore)
			.unlocks(frames[frame].in_flight_fence)
			.done(graphics);

		if (swapchain.present(presentation, frames[frame].render_finished_semaphore, image_index).mustReplace()) {
			recreateSwapchain(device, allocator, surface, window, graphics_ref, presentation_ref, pass, framebuffers, swapchain);
			extent = swapchain.vk_extent;
		}

		frame = (frame + 1) % concurrent_frames;

		// why does this look like this? not 100% sure, ask the linear algebra guy
		sound_system.getListener().position(camera.getPosition()).facing(camera.getDirection() * glm::vec3(-1), camera.getUp());
		sound_system.update();
	}

	device.wait();
	window.close();
	glfwTerminate();

	return 0;
}
