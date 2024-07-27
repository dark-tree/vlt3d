
#include "renderer.hpp"

#include "setup/device.hpp"

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
	SwapchainBuilder builder {VK_PRESENT_MODE_FIFO_KHR, selected, extent, images, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, transform};
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

	// TODO recreate pipelines

}