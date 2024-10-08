#pragma once

#include "external.hpp"

namespace approx {

	/// https://gist.github.com/volkansalma/2972237
	inline float atan2(float y, float x) {
		float abs_y = std::fabs(y) + 1e-10f;      // kludge to prevent 0/0 condition
		float r = (x - std::copysign(abs_y, x)) / (abs_y + std::fabs(x));
		float angle = M_PI/2.f - std::copysign(M_PI/4.f, x);

		angle += (0.1963f * r * r - 0.9817f) * r;
		return std::copysign(angle, y);
	}

}