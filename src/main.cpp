#include "context.hpp"
#include "camera.hpp"

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

	// create the depth buffer
	{
		ImageInfo image_builder {swapchain.vk_extent.width, swapchain.vk_extent.height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT};
		image_builder.preferred(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		image_builder.tiling(VK_IMAGE_TILING_OPTIMAL);

		depth_image = allocator.allocateImage(image_builder);
		depth_image_view = depth_image.getViewBuilder().build(device, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	swapchain = createSwapchain(device, surface, window, graphics, presentation);
	framebuffers = swapchain.getFramebuffers(pass, depth_image_view);

	logger::info("Swapchain recreated!");

}

constexpr const char* vert_shader = R"(
	#version 450

	layout(binding = 0) uniform UniformBufferObject {
		mat4 model;
		mat4 view;
		mat4 proj;
	} uObject;

	layout(location = 0) in vec3 iPosition;
	layout(location = 1) in vec3 iColor;
	layout(location = 2) in vec2 iTexture;

	layout(location = 0) out vec3 vColor;
	layout(location = 1) out vec2 vTexture;

	void main() {
	    gl_Position = uObject.proj * uObject.view * uObject.model * vec4(iPosition, 1.0);
	    vColor = iColor;
		vTexture = iTexture;
	}
)";

constexpr const char* frag_shader = R"(
	#version 450

	layout(binding = 1) uniform sampler2D uSampler;

	layout(location = 0) in vec3 vColor;
	layout(location = 1) in vec2 vTexture;

	layout(location = 0) out vec4 fColor;

	void main() {
		fColor = mix(texture(uSampler, vTexture).rgba, vec4(vColor, 1.0f), 0.5);
	}
)";

struct Vertex {
	float x, y, z, r, g, b, u, v;
};

// for now
#include "world.hpp"

int main() {

	TaskPool pool;

	SoundSystem sound_system;
	SoundBuffer buffer {"assets/sounds/Project_1_mono.ogg"};
	sound_system.add(buffer).loop().play();

	Window window {700, 700, "Funny Vulkan App"};

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
	compiler.setOptimization(shaderc_optimization_level_performance);
	std::future<ShaderModule> vert_mod = pool.defer<ShaderModule>([&] () { return compiler.compile("string_vert", vert_shader, Kind::VERTEX).create(device); });
	std::future<ShaderModule> frag_mod = pool.defer<ShaderModule>([&] () { return compiler.compile("string_frag", frag_shader, Kind::FRAGMENT).create(device); });

	// create VMA based memory allocator
	Allocator allocator {device, instance};

	Atlas atlas = AtlasBuilder::createSimpleAtlas("assets/sprites");

	Chunk chunk {0, 0, 0};
	chunk.random(10000);
	drawChunk(chunk, atlas);

	atlas.getImage().save("atlas.png");

	// vertex buffer
	Buffer vertices;

	{
		BufferInfo buffer_builder{mesh.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
		buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

		vertices = allocator.allocateBuffer(buffer_builder);

		MemoryMap map = vertices.access().map();
		map.write(mesh.data(), mesh.size() * sizeof(Vertex));
		map.flush();
		map.unmap();
	}

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

	// pipeline creation
	GraphicsPipelineBuilder pipe_builder {device};
	pipe_builder.setDynamics(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR);
	pipe_builder.setPrimitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipe_builder.setRenderPass(pass);

	logger::info("Shader compilation took: ", Timer::of([&] {
		pipe_builder.setShaders(vert_mod.get(), frag_mod.get());
	}).milliseconds(), "ms");

	pipe_builder.setDepthTest(VK_COMPARE_OP_LESS, true, true);
	//pipe_builder.setPolygonMode(VK_POLYGON_MODE_LINE);
	//pipe_builder.setLineWidth(3.0f);

	pipe_builder.addBinding()
		.attribute(0, VK_FORMAT_R32G32B32_SFLOAT)
		.attribute(1, VK_FORMAT_R32G32B32_SFLOAT)
		.attribute(2, VK_FORMAT_R32G32_SFLOAT)
		.done();

	VkDescriptorSetLayout descriptor_layout = pipe_builder.addDescriptorSet()
		.descriptor(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.descriptor(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.done();

	GraphicsPipeline pipeline = pipe_builder.build();

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
		image_sampler = image_view.getSamplerBuilder().build(device);
	}

	int concurrent_frames = 2;
	int frame = 0;

	DescriptorPoolBuilder descriptor_pool_builder;
	descriptor_pool_builder.add(concurrent_frames, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	descriptor_pool_builder.add(concurrent_frames, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	DescriptorPool descriptor_pool = descriptor_pool_builder.build(device, concurrent_frames);

	std::vector<Frame> frames;
	std::vector<VkDescriptorSet> sets;

	for (int i = 0; i < concurrent_frames; i ++) {
		frames.emplace_back(allocator, main_pool, device, descriptor_pool.allocate(descriptor_layout), image_sampler);
	}

	Camera camera {window};

	while (!window.shouldClose()) {
		window.poll();
		camera.update();

		Frame& ref = frames[frame];
		ref.data.model = glm::identity<glm::mat4>();
		ref.data.proj = glm::perspective(glm::radians(45.0f), swapchain.vk_extent.width / (float) swapchain.vk_extent.height, 0.1f, 100.0f);
		ref.data.view = camera.getView();

		ref.in_flight_fence.lock();
		ref.map.write(&ref.data, sizeof(UBO));

		uint32_t image_index;
		if (swapchain.getNextImage(frames[frame].image_available_semaphore, &image_index).mustReplace()) {
			recreateSwapchain(device, allocator, surface, window, graphics_ref, presentation_ref, pass, framebuffers, swapchain);
			extent = swapchain.vk_extent;
		}

		// record commands
		frames[frame].buffer.record()
			.beginRenderPass(pass, framebuffers[image_index], extent, 0.0f, 0.0f, 0.0f, 1.0f)
			.bindPipeline(pipeline)
			.bindDescriptorSet(pipeline, frames[frame].set)
			.setDynamicViewport(0, 0, extent.width, extent.height)
			.setDynamicScissors(0, 0, extent.width, extent.height)
			.bindBuffer(vertices)
			.draw(mesh.size())
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
