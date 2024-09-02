
#include "descriptor.hpp"
#include "layout.hpp"
#include "buffer/buffer.hpp"
#include "render/view.hpp"

/*
 * DescriptorSet
 */

DescriptorSet::DescriptorSet(VkDevice device, VkDescriptorSet set, const DescriptorSetLayout* layout)
: vk_device(device), vk_set(set), layout(layout) {}

void DescriptorSet::buffer(int binding, const Buffer& buffer, size_t length, int offset) {

	const VkDescriptorType type = layout->getType(binding);

	VkDescriptorBufferInfo info {};
	info.buffer = buffer.vk_buffer;
	info.offset = offset;
	info.range = length;

	VkWriteDescriptorSet write {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = vk_set;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = type;
	write.descriptorCount = 1;
	write.pBufferInfo = &info;

	vkUpdateDescriptorSets(vk_device, 1, &write, 0, nullptr);

}

void DescriptorSet::sampler(int binding, const ImageSampler& sampler) {

	const VkDescriptorType type = layout->getType(binding);

	VkDescriptorImageInfo info {};
	info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	info.imageView = sampler.vk_view;
	info.sampler = sampler.vk_sampler;

	VkWriteDescriptorSet write {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = vk_set;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = type;
	write.descriptorCount = 1;
	write.pImageInfo = &info;

	vkUpdateDescriptorSets(vk_device, 1, &write, 0, nullptr);

}