
#include "external.hpp"
#include "context.hpp"
#include "client/camera.hpp"
#include "client/immediate.hpp"
#include "client/renderer.hpp"

// for now
#include "world.hpp"
#include "client/gui/stack.hpp"
#include "client/gui/screen/test.hpp"
#include "client/gui/screen/group.hpp"

int main() {

	TaskPool pool;

	SoundSystem sound_system;
	SoundBuffer buffer {"assets/sounds/Project_1_mono.ogg"};
	sound_system.add(buffer).loop().play();

	Window window {1000, 700, "Funny Vulkan App"};
	RenderSystem system {window, 1};

	// for now
	Allocator& allocator = system.allocator;
	Swapchain& swapchain = system.swapchain;
	RenderPass& pass = system.render_pass;

	World world(8888);

	ScreenStack stack;
	ImmediateRenderer renderer {system.assets};
	Camera camera {window};
	camera.move({0, 5, 0});
	window.setRootInputConsumer(&stack);

	// Open the cluster-fuck screen :D
	stack.open(new TestScreen {});

	while (!window.shouldClose()) {
		window.poll();
		camera.update();
		
		std::list<ChunkBuffer>& buffers = world.getBuffers();
		Frame& frame = system.getFrame();

		frame.data.model = glm::identity<glm::mat4>();
		frame.data.proj = glm::perspective(glm::radians(65.0f), swapchain.vk_extent.width / (float) swapchain.vk_extent.height, 0.1f, 1000.0f);
		frame.data.view = camera.getView();

		frame.wait();
		frame.map.write(&frame.data, sizeof(UBO));

		// BEGIN THE "AH YES LET'S JUST OPENGL STYLE IT" SECTION
		world.closeBuffers();
		world.generateAround(camera.getPosition(), 5);
		world.draw(system.assets.getAtlas(), pool, allocator, camera.getPosition(), 8);
		// END THE "AH YES LET'S JUST OPENGL STYLE IT" SECTION

		renderer.prepare(swapchain.vk_extent);
		stack.draw(renderer, window.getInputContext(), camera);
		renderer.write(allocator, frame.immediate_3d, frame.immediate_2d);

		Framebuffer& framebuffer = system.acquireFramebuffer();
		VkExtent2D extent = system.swapchain.vk_extent;

		// record commands
		CommandRecorder commandRecorder = frame.buffer.record()
			.beginRenderPass(pass, framebuffer, extent, 0.0f, 0.0f, 0.0f, 1.0f)
			.bindPipeline(system.pipeline_3d_mix)
			.bindDescriptorSet(frame.set);

		for (auto& buffer : buffers) {
			commandRecorder.bindBuffer(buffer.buffer).draw(buffer.size);
		}

		commandRecorder.bindPipeline(system.pipeline_3d_tint)
			.bindBuffer(frame.immediate_3d.getBuffer())
			.draw(frame.immediate_3d.getCount())
			.bindPipeline(system.pipeline_2d_tint)
			.bindBuffer(frame.immediate_2d.getBuffer())
			.draw(frame.immediate_2d.getCount())
			.endRenderPass()
			.done();

		frame.buffer.submit()
			.awaits(frame.available_semaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			.unlocks(frame.finished_semaphore)
			.unlocks(frame.flight_fence)
			.done(system.graphics_queue);

		system.presentFramebuffer(framebuffer);
		system.nextFrame();

		sound_system.getListener().position(camera.getPosition()).facing(camera.getDirection(), camera.getUp());
		sound_system.update();
	}

	system.wait();
	window.close();
	glfwTerminate();

	return 0;
}
