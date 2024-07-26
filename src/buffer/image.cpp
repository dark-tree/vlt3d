
#include "image.hpp"
#include "buffer.hpp"
#include "allocator.hpp"
#include "command/recorder.hpp"

Image ImageData::upload(Allocator& allocator, CommandRecorder& recorder, VkFormat format) const {

	BufferInfo buffer_builder {size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
	buffer_builder.required(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	buffer_builder.flags(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
	buffer_builder.hint(VMA_MEMORY_USAGE_AUTO_PREFER_HOST);

	Buffer staging = allocator.allocateBuffer(buffer_builder);

	MemoryMap map = staging.access().map();
	map.write(data(), size());
	map.flush();
	map.unmap();

	ImageInfo image_builder {w, h, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT};
	image_builder.preferred(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	image_builder.tiling(VK_IMAGE_TILING_OPTIMAL);
	buffer_builder.hint(VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

	Image image = allocator.allocateImage(image_builder);

	recorder.transitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED);
	recorder.copyBufferToImage(image, staging, w, h);
	recorder.transitionLayout(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	return image;

}
