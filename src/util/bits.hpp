
#pragma once
#include "external.hpp"

/**
 *  Peterson's and Sons
 *      and Friends
 *  Bits & Parts Limited
 */
class Bits {

	public:

		template <typename T>
		class Iterator {

			private:

				T bitfield;
				T current;

			public:

				using value_type = uint8_t;
				using pointer = uint8_t*;
				using reference = uint8_t&;

				Iterator(uint8_t bitfield, uint8_t current)
				: bitfield(bitfield), current(current) {}

				uint8_t operator*() const {
					return current;
				}

				Iterator& operator++() {
					do {
						current >>= 1;
					} while (current != 0 && (bitfield & current) == 0);
					return *this;
				}

				Iterator operator++(int) {
					Iterator tmp = *this;
					++(*this);
					return tmp;
				}

				bool operator==(const Iterator& other) const {
					return current == other.current;
				}

				bool operator!=(const Iterator& other) const {
					return current != other.current;
				}
		};

		template <typename T>
		class Range {

			private:

				T bitfield;

			public:

				Range(T bitfield)
				: bitfield(bitfield) {}

				Iterator<T> begin() const {
					return {bitfield, Bits::msb(bitfield)}; // start with the highest bit
				}

				Iterator<T> end() const {
					return {bitfield, 0}; // end with the "lowest bit"
				}
		};

	public:

		template <std::integral T>
		static Range<T> decompose(T bitfield) {
			return {bitfield};
		}

		/**
		 * Returns the number of bits a specific type uses
		 * allows only singed or unsigned integral types
		 */
		template <std::integral T>
		static inline constexpr T width() {
			return std::numeric_limits<T>::digits + std::numeric_limits<T>::is_signed;
		}

		/**
		 * Returns the given number with all but the MSB
		 * set to zeros, so for 0b0010'0011 will return 0b0010`0000
		 */
		template <std::unsigned_integral T>
		static inline constexpr T msb(T x) {
			static_assert(Bits::width<T>() <= 32, "Bits::msb() works for at most 32 bit types");

			x |= (x >> 1);
			x |= (x >> 2);
			x |= (x >> 4);

			if constexpr (Bits::width<T>() >= 16) {
				x |= (x >> 8);
			}

			if constexpr (Bits::width<T>() >= 32) {
				x |= (x >> 16);
			}

			return x ^ (x >> 1);
		}

		/**
		 * Returns the number of leading zeros in the binary
		 * representation of the given number, so for 0b0010'0011 will return 2
		 */
		template <std::unsigned_integral T>
		static inline constexpr T clz(T v) {
			static_assert(Bits::width<T>() <= 32, "Bits::clz() works for at most 32 bit types");

			T c = Bits::width<T>();
			v &= - static_cast<decltype(std::make_signed<T>())>(v);

			if (v) c--;

			if constexpr (Bits::width<T>() >= 32) {
				if (v & std::bit_cast<T>(0x0000FFFF)) c -= 16;
			}

			if constexpr (Bits::width<T>() >= 16) {
				if (v & std::bit_cast<T>(0x00FF00FF)) c -= 8;
			}

			if (v & std::bit_cast<T>(0x0F0F0F0F)) c -= 4;
			if (v & std::bit_cast<T>(0x33333333)) c -= 2;
			if (v & std::bit_cast<T>(0x55555555)) c -= 1;

			return c;
		}

};