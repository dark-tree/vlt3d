
#pragma once
#include "external.hpp"

class Direction {

	public:

		using field_type = uint8_t;

		//                                         xx yy zz
		static constexpr field_type X          = 0b11'00'00;
		static constexpr field_type WEST       = 0b01'00'00; // -X
		static constexpr field_type EAST       = 0b10'00'00; // +X

		static constexpr field_type Y          = 0b00'11'00;
		static constexpr field_type DOWN       = 0b00'10'00; // -Y
		static constexpr field_type UP         = 0b00'01'00; // +Y

		static constexpr field_type Z          = 0b00'00'11;
		static constexpr field_type NORTH      = 0b00'00'01; // -Z
		static constexpr field_type SOUTH      = 0b00'00'10; // +Z

		static constexpr field_type NEGATIVE   = 0b01'01'01;
		static constexpr field_type POSITIVE   = 0b10'10'10;
		static constexpr field_type HORIZONTAL = 0b11'00'11;
		static constexpr field_type VERTICAL   = 0b00'11'00;
		static constexpr field_type NONE       = 0b00'00'00;
		static constexpr field_type ALL        = 0b11'11'11;

	private:

		field_type value = NONE;

	public:

		Direction() = default;
		Direction(field_type value);
		operator field_type() const;

		/**
		 * Returns the 3D offset (as unit int vector) of a given direction.
		 * Please note that this function accepts a singular direction, so
		 * you may need to use `Bits::decompose()` before calling it
		 */
		static glm::ivec3 offset(Direction direction);

		/**
		 * Returns a direction along the same axis but of
		 * opposite facing, Please note that this function accepts a singular direction, so
		 * you may need to use `Bits::decompose()` before calling it
		 */
		static Direction opposite(Direction direction);

};

