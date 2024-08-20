
#include "attachment.hpp"
#include "setup/debug.hpp"

/*
 * Attachment
 */

void Attachment::allocate(Device& device, VkExtent2D extent, Allocator& allocator) {
	ImageInfo info {extent.width, extent.height, vk_format, vk_usage};
	info.required(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// close if it was already allocated
	close(device);

	allocated = true;
	image = allocator.allocateImage(info);
	view = image.getViewBuilder().build(device, vk_aspect);
	sampler = view.getSamplerBuilder().setFilter(vk_filter).setMode(vk_mode).setBorder(vk_border).build(device);

	image.setDebugName(device, debug_name);
	view.setDebugName(device, debug_name);
	sampler.setDebugName(device, debug_name);
}

void Attachment::close(Device& device) {
	if (allocated) {
		sampler.close(device);
		view.close(device);
		image.close();
		allocated = false;
	}
}

/*
 * AttachmentImageBuilder
 */

AttachmentImageBuilder AttachmentImageBuilder::begin() {
	return {};
}

AttachmentImageBuilder& AttachmentImageBuilder::setFormat(VkFormat format) {
	attachment.vk_format = format;
	return *this;
}

AttachmentImageBuilder& AttachmentImageBuilder::setUsage(VkImageUsageFlags usage) {
	attachment.vk_usage = usage;
	return *this;
}

AttachmentImageBuilder& AttachmentImageBuilder::setAspect(VkImageAspectFlags aspect) {
	attachment.vk_aspect = aspect;
	return *this;
}

AttachmentImageBuilder& AttachmentImageBuilder::setFilter(VkFilter filter) {
	attachment.vk_filter = filter;
	return *this;
}

AttachmentImageBuilder& AttachmentImageBuilder::setMode(VkSamplerAddressMode mode) {
	attachment.vk_mode = mode;
	return *this;
}

AttachmentImageBuilder& AttachmentImageBuilder::setBorder(VkBorderColor border) {
	attachment.vk_border = border;
	return *this;
}

AttachmentImageBuilder& AttachmentImageBuilder::setColorClearValue(float r, float g, float b, float a) {
	attachment.vk_clear.color = {.float32 = {r, g, b, a}};
	return *this;
}

AttachmentImageBuilder& AttachmentImageBuilder::setColorClearValue(int r, int g, int b, int a) {
	attachment.vk_clear.color = {.int32 = {r, g, b, a}};
	return *this;
}

AttachmentImageBuilder& AttachmentImageBuilder::setDepthClearValue(float depth, uint32_t stencil) {
	attachment.vk_clear.depthStencil = {depth, stencil};
	return *this;
}

AttachmentImageBuilder& AttachmentImageBuilder::setDebugName(const char* name) {
	#if !defined(NDEBUG)
	attachment.debug_name = name;
	#endif
	return *this;
}

Attachment AttachmentImageBuilder::build() const {
	return attachment;
}