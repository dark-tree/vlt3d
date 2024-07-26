
#include "resources.hpp"
#include "command/pool.hpp"

ResourceManager::State::State(Device& device, Allocator& allocator, CommandRecorder& recorder)
: atlas(AtlasBuilder::createSimpleAtlas("assets/sprites")), font(8) {
	this->font.addCodePage(atlas, "assets/sprites/8x8font.png", 0);
	this->atlas.getImage().save("atlas.png");

	Image image = atlas.getImage().upload(allocator, recorder, VK_FORMAT_R8G8B8A8_SRGB);
	ImageView view = image.getViewBuilder().build(device, VK_IMAGE_ASPECT_COLOR_BIT);
	this->sampler = view.getSamplerBuilder().setFilter(VK_FILTER_NEAREST).build(device);
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
		} catch (Exception& cause) {
			throw Exception("Failed to reload resources", cause);
		}
		loading.clear();
	} else {
		logger::info("Ignoring resource reload request, it looks like one is already under way");
	}
}
