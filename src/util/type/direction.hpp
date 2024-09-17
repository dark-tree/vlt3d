
#pragma once
#include "external.hpp"
#include "util/math/bits.hpp"

struct DirectionIndex {

	static constexpr int X = 0;
	static constexpr int Y = 1;
	static constexpr int Z = 2;

	static constexpr int WEST  = 0;
	static constexpr int EAST  = 1;
	static constexpr int DOWN  = 2;
	static constexpr int UP    = 3;
	static constexpr int NORTH = 4;
	static constexpr int SOUTH = 5;

};

class Direction {

	public:

		using mask_type = uint8_t;

		//                                      xx yy zz
		static constexpr mask_type X          = 0b11'00'00;
		static constexpr mask_type WEST       = 0b10'00'00; // -X
		static constexpr mask_type EAST       = 0b01'00'00; // +X

		static constexpr mask_type Y          = 0b00'11'00;
		static constexpr mask_type DOWN       = 0b00'10'00; // -Y
		static constexpr mask_type UP         = 0b00'01'00; // +Y

		static constexpr mask_type Z          = 0b00'00'11;
		static constexpr mask_type NORTH      = 0b00'00'10; // -Z
		static constexpr mask_type SOUTH      = 0b00'00'01; // +Z

		// aggregates
		static constexpr mask_type NEGATIVE   = 0b10'10'10;
		static constexpr mask_type POSITIVE   = 0b01'01'01;
		static constexpr mask_type HORIZONTAL = 0b11'00'11;
		static constexpr mask_type VERTICAL   = 0b00'11'00;
		static constexpr mask_type NONE       = 0b00'00'00;
		static constexpr mask_type ALL        = 0b11'11'11;

	public:

		READONLY mask_type mask = NONE;

	public:

		constexpr Direction() = default;
		constexpr Direction(mask_type mask)
		: mask(mask) {}

		constexpr operator mask_type() const {
			return mask;
		}

		/**
		 * Returns a bit range that can be used to iterate
		 * directions combined into a single Direction object
		 * before using methods like `offset()` and `opposite()`.
		 */
		static constexpr Bits::Range<mask_type> decompose(Direction direction) {
			return Bits::decompose<mask_type>(direction.mask);
		}

		/**
		 * Returns the 3D offset (as unit int vector) of a given direction.
		 * Please note that this function accepts a singular direction, so
		 * you may need to use `Direction::decompose()` before calling it
		 */
		static constexpr glm::ivec3 offset(Direction direction) {
			if (direction & WEST) return {-1, 0, 0};
			if (direction & EAST) return {+1, 0, 0};
			if (direction & DOWN) return {0, -1, 0};
			if (direction & UP) return {0, +1, 0};
			if (direction & NORTH) return {0, 0, -1};
			if (direction & SOUTH) return {0, 0, +1};

			return {0, 0, 0};
		}

		/**
		 * Returns a direction along the same axis but of
		 * opposite facing, Please note that this function accepts a singular direction, so
		 * you may need to use `Direction::decompose()` before calling it
		 */
		static constexpr Direction opposite(Direction direction) {
			if (direction & WEST) return EAST;
			if (direction & EAST) return WEST;
			if (direction & DOWN) return UP;
			if (direction & UP) return DOWN;
			if (direction & NORTH) return SOUTH;
			if (direction & SOUTH) return NORTH;

			return NONE;
		}

};

