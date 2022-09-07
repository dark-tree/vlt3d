
#include "vertex.hpp"

VertexAttribute::VertexAttribute( int length, GLenum type, GLsizei size, GLboolean normalize, GLuint divisor ) 
	: 	length(length), 
		type(type), 
		size(size), 
		normalize(normalize),
		divisor(divisor) {

	// noop
}

VertexConsumer::VertexConsumer(GLenum primitive, int vertex_length, int instance_length) 
	: 	primitive(primitive), 
		vertex_length(vertex_length), 
		instance_length(instance_length), 
		vertex_buffer(16), 
		instance_buffer(instance_length != 0 ? 16 : 0) {

	vao = 0;
	vbo = 0;
	ibo = 0;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	// add instance buffer if requesed
	if(instance_length != 0) {
		glGenBuffers(1, &ibo);
	}

	glBindVertexArray(vao);
}

VertexConsumer::VertexConsumer( VertexConsumer&& consumer ) 
	: 	vao(consumer.vao), 
		vbo(consumer.vbo), 
		ibo(consumer.ibo),
		primitive(consumer.primitive), 
		vertex_length(consumer.vertex_length),
		instance_length(consumer.instance_length),
		vertex_buffer(std::move(consumer.vertex_buffer)) {

	consumer.vao = 0;
	consumer.vbo = 0;
	consumer.ibo = 0;
}

VertexConsumer::~VertexConsumer() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void VertexConsumer::bind() {
	glBindVertexArray(vao);
}

void VertexConsumer::submit() {
	this->submitVertexData();

	if( ibo != 0 ) {
		this->submitInstanceData();
	}
}

void VertexConsumer::shrink() {
	this->vertex_buffer.shrink();
	this->instance_buffer.shrink();
}

void VertexConsumer::clear() {
	this->clearInstanceData();
	this->clearVertexData();
}

void VertexConsumer::draw() {
	if( ibo != 0 ) {
		glDrawArraysInstanced(this->primitive, 0, this->vertexCount(), this->instanceCount());
	} else {
		glDrawArrays(this->primitive, 0, this->vertexCount());
	}
}

void VertexConsumerProvider::apply(int& index, Consumer profile) {
	int offset = 0, length = profiles[(int) profile].length;

	for( VertexAttribute attribute : profiles[(int) profile].attributes ) {

		GLHelper::vertexAttribute(
			index, 
			attribute.length,
			attribute.type,
			length,
			offset,
			attribute.size,
			attribute.normalize,
			attribute.divisor
		);

		offset += attribute.length;
		index += 1;

	}

}

void VertexConsumerProvider::setPrimitive( GLenum primitive ) {
	this->primitive = primitive;
}

void VertexConsumerProvider::attribute( int size ) {
	this->attribute( VertexAttribute( size, GL_FLOAT, sizeof(float), GL_FALSE, this->profile == int(Consumer::INSTANCE) ? 1 : 0 ) );
}

void VertexConsumerProvider::attribute( VertexAttribute attr ) {
	this->profiles[this->profile].attributes.push_back(attr);
	this->profiles[this->profile].length += attr.length;
}

void VertexConsumerProvider::target( Consumer profile ) {
	this->profile = (int) profile;
}

VertexConsumer VertexConsumerProvider::get() {
	const int vertex_length = this->profiles[(int) Consumer::VERTEX].length;
	const int instance_length = this->profiles[(int) Consumer::INSTANCE].length;

	VertexConsumer consumer(this->primitive, vertex_length, instance_length);

	// unique attribute id
	int index = 0;

	// bind and configure VBO and IBO
	if( vertex_length != 0 ) {
		glBindBuffer(GL_ARRAY_BUFFER, consumer.vbo);
		this->apply(index, Consumer::VERTEX);
	}

	if( instance_length != 0 ) {
		glBindBuffer(GL_ARRAY_BUFFER, consumer.ibo);
		this->apply(index, Consumer::INSTANCE);
	}

	glBindVertexArray(0);

	return std::move(consumer);
}

