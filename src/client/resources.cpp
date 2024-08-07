
#include "resources.hpp"
#include "command/pool.hpp"
#include "util/threads.hpp"
#include "shader/compiler.hpp"

ResourceManager::State::State(Device& device, Allocator& allocator, CommandRecorder& recorder) {

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

	this->atlas = AtlasBuilder::createSimpleAtlas("assets/sprites", fallback);
	this->font = Font::loadFromFile(atlas, "assets/font.tt");

	atlas.getImage().save("atlas.png");
	fallback.close();

	Image image = atlas.getImage().upload(allocator, recorder, VK_FORMAT_R8G8B8A8_SRGB);
	ImageView view = image.getViewBuilder().build(device, VK_IMAGE_ASPECT_COLOR_BIT);
	this->sampler = view.getSamplerBuilder().setFilter(VK_FILTER_NEAREST).build(device);

	this->vert_2d = compiler.compileFile("assets/shaders/vert_2d.glsl", Kind::VERTEX).create(device);
	this->vert_3d = compiler.compileFile("assets/shaders/vert_3d.glsl", Kind::VERTEX).create(device);
	this->vert_terrain = compiler.compileFile("assets/shaders/vert_terrain.glsl", Kind::VERTEX).create(device);
	this->frag_terrain = compiler.compileFile("assets/shaders/frag_terrain.glsl", Kind::FRAGMENT).create(device);
	this->frag_tint = compiler.compileFile("assets/shaders/frag_tint.glsl", Kind::FRAGMENT).create(device);

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
	return state->sampler;
}

void ResourceManager::reload(Device& device, Allocator& allocator, CommandBuffer buffer) {
	if (!loading.test_and_set()) {
		try {
			CommandRecorder recorder = buffer.record(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			replace(new State(device, allocator, recorder));
			recorder.done();
		} catch (Exception& cause) {
			throw Exception("Failed to reload resources", cause);
		}
		loading.clear();
	} else {
		logger::info("Ignoring resource reload request, it looks like one is already under way");
	}
}
