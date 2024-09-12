#pragma once

#include "external.hpp"

template <typename T, size_t N>
class RingBuffer {

	private:

		int index, count;
		std::array<T, N> buffer;

		using iterator = typename decltype(buffer)::iterator;

		void next() {
			index = (index + 1) % N;
		}

	public:

		using value_type = T;
		static constexpr size_t capacity = N;

		RingBuffer()
		: index(0) {}

		int head() const {
			return index;
		}

		int size() const {
			return count;
		}

		int full() const {
			return count == N;
		}

		int empty() const {
			return count == 0;
		}

	public:

		T& oldest() {
			return buffer[index];
		}

		T& newest() {
			return buffer[(index - 1 + N) % N];
		}

		T& at(int offset) {
			return buffer[offset % N];
		}

		void clear(const T& value) {
			count = 0;
			index = 0;

			for (T& element : buffer) {
				element = value;
			}
		}

		void insert(const T& value) {
			buffer[index] = value;
			count = std::min(count + 1, (int) N);
			next();
		}

		iterator begin() {
			return buffer.begin();
		}

		iterator end() {
			return buffer.end();
		}

};
