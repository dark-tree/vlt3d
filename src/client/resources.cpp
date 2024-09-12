
#include "resources.hpp"
#include "command/pool.hpp"
#include "util/thread/pool.hpp"
#include "shader/compiler.hpp"
#include "client/renderer.hpp"
#include "buffer/array.hpp"

ResourceManager::State::State(RenderSystem& system, TaskQueue& queue, CommandRecorder& recorder)
: device(system.device) {

	Compiler compiler;

	ImageData fallback = ImageData::allocate(8, 8);

	for (int x = 0; x < 8; x ++) {
		for (int y = 0; y < 8; y ++) {
			uint8_t* pixel = fallback.pixel(x, y);
			pixel[0] = ((x < 4) ^ (y < 4)) ? 255 : 0;
			pixel[1] = 0;
			pixel[2] = ((x < 4) ^ (y < 4)) ? 255 : 0;
			pixel[3] = 255;
		}
	}

	this->array = SpriteArray::createFromDirectory(8, 8, "assets/blocks", fallback);
	this->atlas = AtlasBuilder::createSimpleAtlas("assets/sprites", fallback);
	this->font = Font::loadFromFile(atlas, "assets/font.tt");

	atlas.getImage().save("atlas.png");
	array.getImage().save("array.png");
	fallback.close();

	this->atlas_image = atlas.upload(system.allocator, queue, recorder);
	this->atlas_view = atlas_image.getViewBuilder().build(device, VK_IMAGE_ASPECT_COLOR_BIT);
	this->atlas_sampler = atlas_view.getSamplerBuilder().setFilter(VK_FILTER_NEAREST).build(device);

	this->atlas_image.setDebugName(device, "Atlas");
	this->atlas_view.setDebugName(device, "Atlas");
	this->atlas_sampler.setDebugName(device, "Atlas");

	this->array_image = array.upload(system.allocator, queue, recorder);
	this->array_view = array_image.getViewBuilder().setType(VK_IMAGE_VIEW_TYPE_2D_ARRAY).build(device, VK_IMAGE_ASPECT_COLOR_BIT, array.getSpriteCount(), 4); // TODO cleanup
	this->array_sampler = array_view.getSamplerBuilder().setFilter(VK_FILTER_NEAREST).build(device);

	this->array_image.setDebugName(device, "Array");
	this->array_view.setDebugName(device, "Array");
	this->array_sampler.setDebugName(device, "Array");

	this->vert_2d = compiler.compileFile("assets/shaders/vert_2d.glsl", Kind::VERTEX).create(device);
	this->vert_3d = compiler.compileFile("assets/shaders/vert_3d.glsl", Kind::VERTEX).create(device);
	this->vert_terrain = compiler.compileFile("assets/shaders/vert_terrain.glsl", Kind::VERTEX).create(device);
	this->vert_blit = compiler.compileFile("assets/shaders/vert_blit.glsl", Kind::VERTEX).create(device);
	this->vert_occlude = compiler.compileFile("assets/shaders/vert_occlude.glsl", Kind::VERTEX).create(device);
	this->frag_terrain = compiler.compileFile("assets/shaders/frag_terrain.glsl", Kind::FRAGMENT).create(device);
	this->frag_tint = compiler.compileFile("assets/shaders/frag_tint.glsl", Kind::FRAGMENT).create(device);
	this->frag_compose = compiler.compileFile("assets/shaders/frag_compose.glsl", Kind::FRAGMENT).create(device);
	this->frag_ssao = compiler.compileFile("assets/shaders/frag_ssao.glsl", Kind::FRAGMENT).create(device);
	this->frag_occlude = compiler.compileFile("assets/shaders/frag_occlude.glsl", Kind::FRAGMENT).create(device);
}

ResourceManager::State::~State() {
	atlas.close();
	array.close();
	array_sampler.close(device);
	array_view.close(device);
	array_image.close();
	atlas_sampler.close(device);
	atlas_view.close(device);
	atlas_image.close();
	vert_2d.close(device);
	vert_3d.close(device);
	vert_terrain.close(device);
	vert_blit.close(device);
	vert_occlude.close(device);
	frag_terrain.close(device);
	frag_tint.close(device);
	frag_compose.close(device);
	frag_ssao.close(device);
	frag_occlude.close(device);
}

void ResourceManager::replace(State* state) {
	State* old = this->state;
	this->state = state;

	if (old != nullptr) {
		delete old;
	}
}

const Atlas& ResourceManager::getAtlas() const {
	return state->atlas;
}

const Font& ResourceManager::getFont() const {
	return state->font;
}

const ImageSampler& ResourceManager::getAtlasSampler() const {
	return state->atlas_sampler;
}

void ResourceManager::reload(RenderSystem& system, TaskQueue& queue, CommandBuffer buffer) {
	if (!loading.test_and_set()) {
		try {
			CommandRecorder recorder = buffer.record(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			replace(new State(system, queue, recorder));
			recorder.done();
		} catch (Exception& cause) {
			throw Exception("Failed to reload resources", cause);
		}
		loading.clear();
	} else {
		logger::info("Ignoring resource reload request, it looks like one is already under way");
	}
}

void ResourceManager::close() {
	replace(nullptr);
}
