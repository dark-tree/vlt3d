#pragma once

#include "external.hpp"
#include "image.hpp"

class SpriteArray {

	private:

		static constexpr const char* FALLBACK_KEY = "@fallback";

		int width, height;
		ImageData atlas;

		// mutable as when a sprite is missing we need to insert the
		// fallback index so that the warning is only printed once
		mutable std::unordered_map<std::string, uint32_t> sprites;

		int packImage(ImageData image);

	public:

		SpriteArray() = default;
		SpriteArray(int width, int height);

		void submitFile(const std::string& identifier, const std::string& path);
		void submitImage(const std::string& identifier, ImageData image);

		/**
		 * Returns the layer number of the given sprite, will return the
		 * layer of the fallback sprite if the given identifier is missing
		 */
		uint32_t getSpriteIndex(const std::string& identifier) const;

		/**
		 * Returns the number of sprites in the array, as the sprites are always tightly
		 * packed in the image the used region has height equal sprite_height * sprite_count
		 */
		uint32_t getSpriteCount() const;

		const ImageData& getImage() const;
		Image upload(Allocator& allocator, TaskQueue& queue, CommandRecorder& recorder) const;

		void close();

	public:

		static SpriteArray createFromDirectory(int width, int height, const std::string& root, ImageData fallback);

};
