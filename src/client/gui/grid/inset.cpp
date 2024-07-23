
#include "inset.hpp"

Inset::Inset()
: Inset(0, 0, 0, 0) {}

Inset::Inset(float top, float bottom, float left, float right)
: top(top), bottom(bottom), left(left), right(right) {}
