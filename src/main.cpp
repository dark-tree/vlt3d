
#include "external.hpp"
#include "context.hpp"
#include "client/camera.hpp"
#include "client/immediate.hpp"
#include "client/renderer.hpp"

// for now
#include "world/world.hpp"
#include "world/renderer.hpp"
#include "world/generator.hpp"
#include "client/frustum.hpp"
#include "client/gui/stack.hpp"
#include "client/gui/screen/test.hpp"
#include "client/gui/screen/group.hpp"
#include "window/profiler.hpp"
#include "client/gui/screen/play.hpp"
#include "world/skybox.hpp"

struct GeometryPushBlock {
	glm::mat4 mvp;
	glm::mat4 view;
};

struct LightingPushBlock {
	glm::mat4 projection;
	Sun sun;
	char pad [3*4*4];
};

int main() {

	TaskPool pool;

	SoundSystem sound_system;
	SoundBuffer buffer {"assets/sounds/Project_1_mono.ogg"};
//	sound_system.add(buffer).loop().play();

	Window window {1000, 700, "Funny Vulkan App"};
	RenderSystem system {window, 1};

	// for now
	Swapchain& swapchain = system.swapchain;

	World world;
	WorldRenderer world_renderer {system, world};
	WorldGenerator world_generator {8888};

	ScreenStack stack;
	ImmediateRenderer immediate {system.assets};
	Camera camera {window};
	camera.move({0, 5, 0});
	window.setRootInputConsumer(&stack);

	Profiler profiler;
	stack.open(new GroupScreen {new PlayScreen {world, camera}, new TestScreen {profiler}});

	while (!window.shouldClose()) {
		window.poll();
		profiler.next();
		
		Frame& frame = system.getFrame();

		glm::mat4 view = camera.getView();
		glm::mat4 light = glm::inverse(view);
		glm::mat4 projection = glm::perspective(glm::radians(65.0f), swapchain.vk_extent.width / (float) swapchain.vk_extent.height, 0.1f, 1000.0f);

		glm::mat4 mvp = projection * view;
		Frustum frustum = camera.getFrustum(projection);

		frame.wait();
		frame.execute();

		immediate.prepare(swapchain.vk_extent);
		stack.draw(system, immediate, window.getInputContext(), camera);

		Framebuffer& framebuffer = system.acquireScreenFramebuffer();
		VkExtent2D extent = system.swapchain.vk_extent;

		// record commands
		CommandRecorder recorder = frame.buffer.record();

		immediate.write(system, frame.immediate_3d, frame.immediate_2d);
		frame.immediate_2d.upload(recorder);
		frame.immediate_3d.upload(recorder);
		world_renderer.prepare(recorder);

		// TODO?
		// wait for all pending buffer uploads, doing it here maybe is stupid, hard to tell
		// it sounds like it would be better to do it after the static buffer have been drawn
		// just before we actually need the transfers to finish but then it would be inside a render pass
		// so we would need some internal subpass dependency stuff that i know nothing about
		recorder.bufferTransferBarrier();

		world.update(world_generator, camera.getPosition(), 16);

		GeometryPushBlock geometry_push_block {};
		geometry_push_block.mvp = mvp;
		geometry_push_block.view = view;

		recorder.beginRenderPass(system.terrain_pass, system.terrain_framebuffer, extent)
			.bindPipeline(system.pipeline_3d_terrain)
			.writePushConstant(system.push_constant, &geometry_push_block)
			.bindDescriptorSet(frame.set_0);

		world_renderer.draw(recorder, frustum);
		world_renderer.eraseOutside(camera.getPosition(), 17);

		recorder.endRenderPass();

		Skybox skybox;
		LightingPushBlock lighting_push_block {};
		lighting_push_block.projection = projection;
		lighting_push_block.sun = skybox.getSunData(0);

		recorder.beginRenderPass(system.ssao_pass, system.ssao_framebuffer, extent)
			.bindPipeline(system.pipeline_ssao)
			.writePushConstant(system.push_constant, &lighting_push_block)
			.bindDescriptorSet(frame.set_2)
			.draw(3) // draw blit quad
			.endRenderPass();

		lighting_push_block.projection = light;

		recorder.beginRenderPass(system.lighting_pass, framebuffer, extent)
			.bindPipeline(system.pipeline_compose)
			.bindDescriptorSet(frame.set_3)
			.writePushConstant(system.push_constant, &lighting_push_block)
			.draw(3);

		recorder.nextSubpass()
			.bindPipeline(system.pipeline_3d_tint)
			.bindDescriptorSet(frame.set_1)
			.writePushConstant(system.push_constant, &geometry_push_block)
			.bindBuffer(frame.immediate_3d.getBuffer())
			.draw(frame.immediate_3d.getCount())
			.bindPipeline(system.pipeline_2d_tint)
			.bindBuffer(frame.immediate_2d.getBuffer())
			.draw(frame.immediate_2d.getCount())
			.endRenderPass();

		recorder.done();

		frame.buffer.submit()
			.awaits(frame.available_semaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
			.unlocks(frame.finished_semaphore)
			.unlocks(frame.flight_fence)
			.done(system.graphics_queue);

		system.presentScreenFramebuffer(framebuffer);
		system.nextFrame();

		sound_system.getListener().position(camera.getPosition()).facing(camera.getDirection(), camera.getUp());
		sound_system.update();
	}

	world_renderer.close();
	system.close();
	window.close();
	glfwTerminate();

	return 0;
}
