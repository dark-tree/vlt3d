#pragma once

#include "external.hpp"
#include "image.hpp"
#include "patch.hpp"
#include "sprites.hpp"

class Atlas {

	private:

		ImageData atlas;
		PairedSprite fallback;
		mutable std::unordered_map<std::string, PairedSprite> sprites;
		friend class AtlasBuilder;

		Atlas(ImageData atlas, const std::unordered_map<std::string, UnbakedSprite>& unbaked, PairedSprite fallback);
		const PairedSprite& getSpritePair(const std::string& identifier) const;

	public:

		Atlas() = default;

		BakedSprite getBakedSprite(const std::string& identifier) const;
		UnbakedSprite getUnbakedSprite(const std::string& identifier) const;
		NinePatch getNinePatch(const std::string& identifier, int m) const;
		const ImageData& getImage() const;
		Image upload(Allocator& allocator, TaskQueue& queue, CommandRecorder& recorder) const;

		void close();

};

class AtlasBuilder {

	private:

		static constexpr const char* fallback_key = "@fallback";

		ImageData atlas;
		std::unordered_map<std::string, UnbakedSprite> sprites;

		bool canPlaceAt(int x, int y, int w, int h);
		UnbakedSprite packUnbakedSprite(ImageData image);
		PairedSprite getFallback() const;

	public:

		AtlasBuilder();

		void submitFile(const std::string& identifier, const std::string& path);
		void submitImage(const std::string& identifier, ImageData image);
		void submitFallback(ImageData image);
		Atlas build();

	public:

		static Atlas createSimpleAtlas(const std::string& identifier, ImageData fallback);
		static Atlas createIdentityAtlas(ImageData atlas);

};
