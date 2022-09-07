#pragma once

#include <include/external.hpp>
#include <include/forward.hpp>
#include <util/buffer.hpp>

#define HAS_SSBO 1
#define HAS_UBO 1
#define MAX_SIZE 128

template< typename T >
class StorageBuffer {

	private:

		GLuint object;
		GLenum type;
		int binding;
		unsigned int size;

		bool dirty = true;
		
		FastBuffer<T> buffer;

	public:

		StorageBuffer( int binding, int size_hint = -1 ) {
			glGenBuffers(1, &object);
			this->binding = binding;

			if( HAS_SSBO ) {
				size = -1;
				type = GL_SHADER_STORAGE_BUFFER;
			}else{
				if( !HAS_UBO ) {
					throw std::runtime_error("Uniform array fallback not yet implemented!");
				}

				if( size_hint == -1 ) {
					size = MAX_SIZE * sizeof(T);
				}else{
					size = size_hint * sizeof(T);
				}
				type = GL_UNIFORM_BUFFER;
			}
		}

		StorageBuffer( StorageBuffer&& buffer ) : object(buffer.object), type(buffer.type), binding(buffer.binding), size(buffer.size), buffer(std::move(buffer.buffer)) {
			buffer.object = 0;
		}

		~StorageBuffer() {
			glDeleteBuffers(1, &object);
		}

		void push( const T& value ) {
			this->dirty = true;
			this->buffer.insert(value);
		}

		T& get( int index ) {
			this->dirty = true;
			return this->peek(index);
		}

		T& peek( int index ) {
			return this->buffer.read(index);
		}

		void remove( int index ) {
			this->dirty = true;
			this->buffer.remove(index);
		}

		int count() {
			return this->buffer.count();
		}

		void clear() {
			this->dirty = true;
			this->buffer.clear();
		}

		// prepare and bind OpenGL buffers
		void bind() {
			glBindBuffer(this->type, this->object);
			glBindBufferBase(this->type, this->binding, this->object);
		}

		void base(int binding) {
			this->binding = binding;
		}

		void submit() {
			this->bind();

			if( this->dirty ) {
				glBufferData(this->type, std::min((unsigned int)this->buffer.size(), this->size), this->buffer.data(), GL_DYNAMIC_DRAW);
			}
		}

};

