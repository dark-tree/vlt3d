
#include "external.hpp"
#include "context.hpp"
#include "client/camera.hpp"
#include "client/immediate.hpp"
#include "client/renderer.hpp"

// for now
#include "world.hpp"
#include "buffer/font.hpp"
#include "client/gui/stack.hpp"
#include "client/gui/screen/test.hpp"
#include "client/gui/screen/group.hpp"
#include "client/resources.hpp"

int main() {

	TaskPool pool;

	SoundSystem sound_system;
	SoundBuffer buffer {"assets/sounds/Project_1_mono.ogg"};
	sound_system.add(buffer).loop().play();

	Window window {1000, 700, "Funny Vulkan App"};
	RenderSystem system {pool, window, 1};

	// for now
	Device& device = system.device;
	Allocator& allocator = system.allocator;
	VkQueue& graphics = system.graphics_queue;
	VkQueue& presentation_queue = system.presentation_queue;
	Swapchain& swapchain = system.swapchain;
	RenderPass& pass = system.render_pass;
	std::vector<Framebuffer>& framebuffers = system.framebuffers;

	World world(8888);

	auto extent = swapchain.vk_extent;

	ScreenStack stack;
	ImmediateRenderer renderer {system.assets};
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

		Frame& ref = system.getFrame();

		ref.data.model = glm::identity<glm::mat4>();
		ref.data.proj = glm::perspective(glm::radians(65.0f), swapchain.vk_extent.width / (float) swapchain.vk_extent.height, 0.1f, 1000.0f);
		ref.data.view = camera.getView();

		ref.map.write(&ref.data, sizeof(UBO));

		// BEGIN THE "AH YES LET'S JUST OPENGL STYLE IT" SECTION
		// * Horribly inefficient
		// * Incompatible with threading and concurrent frames
		world.closeBuffers();
		world.generateAround(camera.getPosition(), 5);
		world.draw(system.assets.getAtlas(), pool, allocator, camera.getPosition(), 8);

		renderer.prepare(swapchain.vk_extent);
		stack.draw(renderer, window.getInputContext(), camera);
		renderer.write(allocator, buffer_3d, buffer_2d);
		// * Also we now have 3 separate buffer types
		// END THE "AH YES LET'S JUST OPENGL STYLE IT" SECTION

		uint32_t image_index;
		if (swapchain.getNextImage(ref.image_available_semaphore, &image_index).mustReplace()) {
			system.recreateSwapchain();
			extent = swapchain.vk_extent;
		}

		// record commands
		CommandRecorder commandRecorder = ref.buffer.record()
			.beginRenderPass(pass, framebuffers[image_index], extent, 0.0f, 0.0f, 0.0f, 1.0f)
			.bindPipeline(system.pipeline_3d_mix)
			.bindDescriptorSet(ref.set)
			.setDynamicViewport(0, 0, extent.width, extent.height)
			.setDynamicScissors(0, 0, extent.width, extent.height);

		for (auto& buffer : buffers) {
			commandRecorder.bindBuffer(buffer.buffer).draw(buffer.size);
		}

		commandRecorder.bindPipeline(system.pipeline_3d_tint)
			.bindBuffer(buffer_3d.getBuffer())
			.draw(buffer_3d.getCount())
			.bindPipeline(system.pipeline_2d_tint)
			.bindBuffer(buffer_2d.getBuffer())
			.draw(buffer_2d.getCount())
			.endRenderPass()
			.done();

		ref.buffer.submit()
			.awaits(ref.image_available_semaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			.unlocks(ref.render_finished_semaphore)
			.unlocks(ref.in_flight_fence)
			.done(graphics);

		if (swapchain.present(presentation_queue, ref.render_finished_semaphore, image_index).mustReplace()) {
			system.recreateSwapchain();
			extent = swapchain.vk_extent;
		}

		system.nextFrame();

		// TODO audio is busted irit
		// why does this look like this? not 100% sure, ask the linear algebra guy
		sound_system.getListener().position(camera.getPosition()).facing(camera.getDirection() * glm::vec3(-1), camera.getUp());
		sound_system.update();
	}

	device.wait();
	window.close();
	glfwTerminate();

	return 0;
}
