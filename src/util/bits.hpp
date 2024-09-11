
#pragma once
#include "external.hpp"

/**
 *  Peterson's and Sons
 *      and Friends
 *  Bits & Parts Limited
 */
class Bits {

	public:

		template <std::unsigned_integral T>
		class Iterator {

			private:

				T bitfield;
				T current;

			public:

				using value_type = T;
				using pointer = T*;
				using reference = T&;

				Iterator(T bitfield, T current)
				: bitfield(bitfield), current(current) {}

				T operator*() const {
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

		template <std::unsigned_integral T>
		class Range {

			private:

				T bitfield;

			public:

				Range(T bitfield)
				: bitfield(bitfield) {}

				Iterator<T> begin() const {
					return {bitfield, Bits::msbMask<T>(bitfield)}; // start with the highest bit
				}

				Iterator<T> end() const {
					return {bitfield, 0}; // end with the "lowest bit"
				}

		};

	public:

		template <std::unsigned_integral T>
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
		static inline constexpr T msbMask(T x) {
			static_assert(Bits::width<T>() <= 32, "Bits::msbMask() works for at most 32 bit types");

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
		* Returns the index of the MSB counting from 0, from the right to the left
		* so for 0b0010'0011 will return 5, passing zero as input will return 0
		*/
		template <std::unsigned_integral T>
		static inline int msbIndex(T value) {
			int index = 0;

			while (value >>= 1) {
				index ++;
			}

			return index;
		}

};