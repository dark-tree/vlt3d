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

void recreateSwapchain(Device& device, WindowSurface& surface, Window& window, QueueInfo& graphics, QueueInfo& presentation, RenderPass& pass, std::vector<Framebuffer>& framebuffers, Swapchain& swapchain) {

	device.wait();
	swapchain.close();

	for (Framebuffer& framebuffer : framebuffers) {
		framebuffer.close();
	}

	swapchain = createSwapchain(device, surface, window, graphics, presentation);
	framebuffers = swapchain.getFramebuffers(pass);

	logger::info("Swapchain recreated!");

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
	builder.addDebugMessenger();

	// instance and surface creation, and device selection
	Instance instance = builder.build();
	WindowSurface surface = instance.createSurface(window);
	DeviceBuilder device_builder = pickDevice(instance, surface);

	// device configuration
	QueueInfo graphics_ref = device_builder.addQueue(VK_QUEUE_GRAPHICS_BIT, 1);
	QueueInfo presentation_ref = device_builder.addQueue(surface, 1);
	device_builder.addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	device_builder.features.enableFillModeNonSolid().orFail();
	device_builder.features.enableWideLines().orFail();

	// device and queue creation
	Device device = device_builder.create();
	VkQueue graphics = device.get(graphics_ref, 0);
	VkQueue presentation = device.get(presentation_ref, 0);

	// swapchain creation
	Swapchain swapchain = createSwapchain(device, surface, window, graphics_ref, presentation_ref);
	auto extent = swapchain.vk_extent;

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
	std::vector<Framebuffer> framebuffers = swapchain.getFramebuffers(pass);

	// pipeline creation
	GraphicsPipelineBuilder pipe_builder {device};
	pipe_builder.setDynamics(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR);
	pipe_builder.setPrimitive(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipe_builder.setRenderPass(pass);
	pipe_builder.setShaders(vert_mod, frag_mod);
	pipe_builder.setPolygonMode(VK_POLYGON_MODE_LINE);
	pipe_builder.setLineWidth(3.0f);

	GraphicsPipeline pipeline = pipe_builder.build();

	// create command buffer
	CommandPool pool = CommandPool::build(device, graphics_ref, false);
	CommandBuffer buffer = pool.allocate();

	Semaphore image_available_semaphore = device.semaphore();
	Semaphore render_finished_semaphore = device.semaphore();
	Fence in_flight_fence = device.fence(true);

	while (!window.shouldClose()) {
		window.poll();

		in_flight_fence.wait();

		uint32_t image_index;
		if (swapchain.getNextImage(image_available_semaphore, &image_index).mustReplace()) {
			recreateSwapchain(device, surface, window, graphics_ref, presentation_ref, pass, framebuffers, swapchain);
			extent = swapchain.vk_extent;
		}

		in_flight_fence.reset();

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

		if (swapchain.present(presentation, render_finished_semaphore, image_index).mustReplace()) {
			recreateSwapchain(device, surface, window, graphics_ref, presentation_ref, pass, framebuffers, swapchain);
			extent = swapchain.vk_extent;
		}


	}

	device.wait();
	window.close();
	glfwTerminate();

	return 0;
}
