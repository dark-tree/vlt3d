#pragma once

#include "logger.hpp"
#include "external.hpp"
#include "inline.hpp"

template <typename T> requires std::is_trivially_copyable_v<T>
class ArrayBuffer {

	private:

		T* array;
		size_t offset;
		size_t length;

	public:

		ArrayBuffer() : ArrayBuffer(1024) {}
		ArrayBuffer(ArrayBuffer<T>& buffer) = delete;

		ArrayBuffer(size_t length) {
			this->offset = 0;
			this->length = length;
			this->array = (T*) malloc(length * sizeof(T));
		}

		ArrayBuffer(ArrayBuffer<T>&& buffer) {
			this->offset = buffer.offset;
			this->length = buffer.length;
			this->array = buffer.array;

			buffer.array = nullptr;
		}

		~ArrayBuffer() {
			if (array != nullptr) {
				free(array);
			}

			array = nullptr;
		}

		FORCE_INLINE void push(T value) {
			array[offset ++] = value;
		}

		FORCE_INLINE void push(T* buffer, size_t length) {
			memcpy(array, buffer, length * sizeof(T));
			offset += length;
		}

		FORCE_INLINE void clear() {
			offset = 0;
		}

		FORCE_INLINE void prepare(size_t count) {
			if (offset + count > length) {
				const size_t expanded = length * 2;

				logger::info("Buffer ", this, " resized to ", expanded, " (", expanded * sizeof(T), " bytes)");

				this->array = (T*) realloc(array, expanded * sizeof(T));
				this->length = expanded;
			}
		}

		FORCE_INLINE T* data() {
			return data;
		}

		FORCE_INLINE const T* data() const {
			return data;
		}

		FORCE_INLINE T* copy() const {
			T* buffer = new T[offset];
			memcpy(buffer, array, offset * sizeof(T));

			return buffer;
		}

		FORCE_INLINE size_t size() const {
			return offset;
		}

		FORCE_INLINE bool empty() const {
			return offset == 0;
		}

};