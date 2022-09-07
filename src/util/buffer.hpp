#pragma once

#include "trait.hpp"
#include "logger.hpp"
#include "allocator.hpp"

template< typename T, class Allocator = DefaultAllocator<T> >
class FastBuffer {

	private:

		T* buffer;
		int pos;
		int length;

	protected:

		void push( T value ) {
			this->buffer[ this->pos ++ ] = value;
		}

		template< typename... Args, typename = trait::are_types_equal< T, Args... > >
		void push( T first, Args... args ) {
			this->push(first);
			this->push(args...);
		}

	public:

		FastBuffer( int size = 16 );
		FastBuffer( FastBuffer&& );
		~FastBuffer();

		// copy buffer into a new array and return pointer
		T* copy();

		// get pointer to internal continous buffer
		T* data();

		// get pointer to element
		T* data( int index );

		// copy data from other FastBuffer
		void from(FastBuffer<T, Allocator>& buffer);

		// get internal buffer size in bytes
		int size();

		// check if this buffer is empty
		bool empty();
		
		// get number of elements in this buffer
		int count();

		// get current maximum capacity
		int capacity();

		// clear this buffer
		void clear();

		// shrinks the buffer to fit data
		void shrink();
		
		// ensure that `count` elements can be inserted
		void reserve( int count );

		// get reference of element at `index` to buffer element
		T& read( int index );

		// write `value` into buffer at `index`
		void write( int index, T value );

		// remove element at `index` from buffer
		void remove( int index );

		// insert `value` into the buffer
		void insert( T value );

		// insert `array` into buffer
		void insert( T* array, int length );

		// insert multiple elements into buffer
		template< typename... Args, typename = trait::are_types_equal< T, Args... > >
		void insert( T first, Args... args ) {
			this->reserve( 1 + sizeof...(args) );
			this->push(first);
			this->push(args...);
		}

};

template< typename T, class Allocator >
FastBuffer<T, Allocator>::FastBuffer( int length ) {
	this->pos = 0;
	this->length = length;
	this->buffer = (T*) Allocator::malloc(length);
}

template< typename T, class Allocator >
FastBuffer<T, Allocator>::FastBuffer( FastBuffer<T, Allocator>&& buffer ) {
	this->pos = buffer.pos;
	this->length = buffer.length;
	this->buffer = buffer.buffer;

	buffer.buffer = nullptr;
}

template< typename T, class Allocator >
FastBuffer<T, Allocator>::~FastBuffer() {
	if( this->buffer != nullptr ) {
		free(this->buffer);
	}
}

template< typename T, class Allocator >
T* FastBuffer<T, Allocator>::copy() {
	T* buf = new T[this->pos];
	memcpy(buf, this->buffer, this->pos * sizeof(T));

	return buf;
}

template< typename T, class Allocator >
T* FastBuffer<T, Allocator>::data() {
	return this->buffer;
}

template< typename T, class Allocator >
T* FastBuffer<T, Allocator>::data( int index ) {
	return this->buffer + index;
}

template< typename T, class Allocator >
void FastBuffer<T, Allocator>::from(FastBuffer<T, Allocator>& buffer) {
	this->insert(buffer.data(), buffer.count());
}

template< typename T, class Allocator >
int FastBuffer<T, Allocator>::size() {
	return this->pos * sizeof(T);
}

template< typename T, class Allocator >
bool FastBuffer<T, Allocator>::empty() {
	return this->pos == 0;
}

template< typename T, class Allocator >
int FastBuffer<T, Allocator>::count() {
	return this->pos;
}

template< typename T, class Allocator >
int FastBuffer<T, Allocator>::capacity() {
	return this->length;
}

template< typename T, class Allocator >
void FastBuffer<T, Allocator>::clear() {
	this->pos = 0;
}

template< typename T, class Allocator >
void FastBuffer<T, Allocator>::shrink() {
	if( this->length > this->pos ) {

#		if LT3D_VERBOSE_FAST_BUFFER == true
		logger::info("Buffer ", this, " resized to ", this->pos, " (", this->pos * sizeof(T), " bytes)");
#		endif		

		this->buffer = (T*) Allocator::realloc(this->buffer, this->pos);
		this->length = this->pos;
	}
}

template< typename T, class Allocator >
void FastBuffer<T, Allocator>::reserve( int count ) {
	while( this->pos + count > this->length ) {
		int new_size = this->length * 2;

#		if LT3D_VERBOSE_FAST_BUFFER == true
		logger::info("Buffer ", this, " resized to ", new_size, " (", new_size * sizeof(T), " bytes)");
#		endif

		this->buffer = (T*) Allocator::realloc(this->buffer, new_size);
		this->length = new_size;
	}
}

template< typename T, class Allocator >
T& FastBuffer<T, Allocator>::read( int index ) {
	return this->buffer[index];
}

template< typename T, class Allocator >
void FastBuffer<T, Allocator>::write( int index, T value ) {
	return this->buffer[index] = value;
}

template< typename T, class Allocator >
void FastBuffer<T, Allocator>::remove( int index ) {
	std::copy(this->buffer + index + 1, this->buffer + this->pos, this->buffer + index);
	this->pos --;
}

template< typename T, class Allocator >
void FastBuffer<T, Allocator>::insert( T data ) {
	this->reserve(1);
	this->push(data);
}

template< typename T, class Allocator >
void FastBuffer<T, Allocator>::insert( T* array, int length ) {
	this->reserve(length);
	memcpy((this->buffer + this->pos), array, length * sizeof(T));

	this->pos += length;
}

