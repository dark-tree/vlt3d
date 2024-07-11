#pragma once

#include "external.hpp"
#include "buffer/buffer.hpp"
#include "buffer/allocator.hpp"
#include "buffer/font.hpp"

class ScreenRenderer {

	public:

		void getBuffers(Allocator& allocator, Buffer* buf1, int* len1, Buffer* buf2, int* len2, Font& font);

};