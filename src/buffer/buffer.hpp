#pragma once

#include "external.hpp"
#include "setup/device.hpp"
#include "memory.hpp"

class Buffer {

	public:

		READONLY VkBuffer vk_buffer;
		READONLY MemoryAccess memory;

	public:

		Buffer() {}

		Buffer(VkBuffer vk_buffer, const MemoryAccess& memory)
		: vk_buffer(vk_buffer), memory(memory) {}

		MemoryAccess& access() {
			return memory;
		}

		void close() {
			memory.closeBuffer(vk_buffer);
		}

};
