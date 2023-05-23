#include "glph.hpp"
#include "context.hpp"

DeviceBuilder pickDevice(Instance& instance, WindowSurface& surface) {
	for (DeviceInfo& device : instance.getDevices()) {
		if (device.getQueueFamily(VK_QUEUE_GRAPHICS_BIT) && device.getQueueFamily(surface) && device.hasSwapchain(surface)) {
			logger::info("Selected device: ", device.getProperties().deviceName);
			return device.builder();
		}
	}

	throw std::runtime_error("No viable vulkan device found!");
}

constexpr const char* vert_shader = R"(
	#version 450

	layout(location = 0) out vec3 fragColor;

	vec2 positions[3] = vec2[](
		vec2(0.0, -0.5),
		vec2(0.5, 0.5),
		vec2(-0.5, 0.5)
	);

	vec3 colors[3] = vec3[](
		vec3(1.0, 0.0, 0.0),
		vec3(0.0, 1.0, 0.0),
		vec3(0.0, 0.0, 1.0)
	);

	void main() {
		gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
		fragColor = colors[gl_VertexIndex];
	}
)";

constexpr const char* frag_shader = R"(
	#version 450

	layout(location = 0) in vec3 fragColor;

	layout(location = 0) out vec4 outColor;

	void main() {
		outColor = vec4(fragColor, 1.0);
	}
)";

int main() {

    glfwInit();
	Window window {800, 600, "Funny Vulkan App"};

	// instance configuration
	InstanceBuilder builder;
	builder.addApplicationInfo("My Funny Vulkan Application");
	builder.addValidationLayer("VK_LAYER_KHRONOS_validation").orFail();
	//builder.addValidationLayer("VK_LAYER_LUNARG_api_dump").orFail();
	builder.addDebugMessenger();

	// instance and surface creation, and device selection
	Instance instance = builder.build();
	WindowSurface surface = instance.createSurface(window);
	DeviceBuilder device_builder = pickDevice(instance, surface);

	// device configuration
	QueueInfo graphics_ref = device_builder.addQueue(VK_QUEUE_GRAPHICS_BIT, 1);
	QueueInfo presentation_ref = device_builder.addQueue(surface, 1);
	device_builder.addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	// device and queue creation
	Device device = device_builder.create({});
	VkQueue graphics = device.get(graphics_ref, 0);
	VkQueue presentation = device.get(presentation_ref, 0);

	// swapchain information gathering
	SwapchainInfo info {device.vk_physical_device, surface};
	auto extent = info.getExtent(window);
	auto images = info.getImageCount();
	auto transform = info.getTransform();

	VkSurfaceFormatKHR selected_format = info.getFormats()[0];

	for (auto& f : info.getFormats()) {
		if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			selected_format = f;
		}
	}

	// swapchain creation
	SwapchainBuilder swapchain_builder {VK_PRESENT_MODE_FIFO_KHR, selected_format, extent, images, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, transform};
	swapchain_builder.addSyncedQueue(graphics_ref);
	swapchain_builder.addSyncedQueue(presentation_ref);
	Swapchain swapchain = swapchain_builder.build(device, surface);

	// get images from the swapchain
	std::vector<ImageView> image_views;
	for (Image& image : swapchain.getImages()) {
		image_views.push_back(image.builder().build(device));
	}

	// create a compiler and compile the glsl into spirv
	Compiler compiler;
	compiler.setOptimization(shaderc_optimization_level_performance);
	ShaderModule vert_mod = compiler.compile("string_vert", vert_shader, Kind::VERTEX).create(device);
	ShaderModule frag_mod = compiler.compile("string_frag", frag_shader, Kind::FRAGMENT).create(device);

	// render pass creation
	RenderPassBuilder pass_builder;

	pass_builder.addAttachment(swapchain.vk_surface_format.format, VK_SAMPLE_COUNT_1_BIT)
		.input(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED)
		.output(VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		.next();

	pass_builder.addDependency()
		.input(VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0)
		.output(0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
		.next();

	pass_builder.addSubpass(VK_PIPELINE_BIND_POINT_GRAPHICS)
		.addColor(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		.next();

	RenderPass pass = pass_builder.build(device);

	// create framebuffers
	std::vector<Framebuffer> framebuffers;
	for (ImageView& view : image_views) {
		framebuffers.push_back(Framebuffer::build(device, pass, view, extent.width, extent.height));
	}

	// pipeline creation
	GraphicsPipelineBuilder pipe_builder;
	pipe_builder.setDynamics(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR);
	pipe_builder.setPrimitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipe_builder.setRenderPass(pass);
	pipe_builder.setShaders(vert_mod, frag_mod);

	GraphicsPipeline pipeline = pipe_builder.build(device);

	// create command buffer
	CommandPool pool = CommandPool::build(device, graphics_ref, false);
	CommandBuffer buffer = pool.allocate();

	Semaphore image_available_semaphore {device.vk_device};
	Semaphore render_finished_semaphore {device.vk_device};
	Fence in_flight_fence {device.vk_device, true};

	while (!window.shouldClose()) {
		window.poll();

		in_flight_fence.lock();

		uint32_t image_index = swapchain.getNextImage(image_available_semaphore);

		// record commands
		buffer.record()
			.beginRenderPass(pass, framebuffers[image_index], extent, 0.0f, 0.0f, 0.0f, 1.0f)
			.bindPipeline(pipeline)
			.setDynamicViewport(0, 0, extent.width, extent.height)
			.setDynamicScissors(0, 0, extent.width, extent.height)
			.draw(3)
			.endRenderPass()
			.done();

		buffer.submit()
			.awaits(image_available_semaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			.unlocks(render_finished_semaphore)
			.onFinished(in_flight_fence)
			.done(graphics);

		swapchain.present(presentation, render_finished_semaphore, image_index);

	}

	device.wait();
	window.close();
	glfwTerminate();

	return 0;
}
