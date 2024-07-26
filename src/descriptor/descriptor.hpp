#pragma once

#include "external.hpp"

class DescriptorSet {

	public:

		READONLY VkDevice vk_device;
		READONLY VkDescriptorSet vk_set;

	public:

		DescriptorSet() {}

		DescriptorSet(VkDevice device, VkDescriptorSet set)
		: vk_device(device), vk_set(set) {}

		void buffer(int binding, VkDescriptorType type, Buffer& buffer, size_t length, int offset = 0) {

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

		void sampler(int binding, VkDescriptorType type, const ImageSampler& sampler) {

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

};