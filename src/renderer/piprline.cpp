
#include "pipeline.hpp"

Pipeline& Pipeline::setShader(ShaderProgram* shader) {
	this->shader = shader;

	return *this;
}

Pipeline& Pipeline::setConsumer(VertexConsumer* consumer) {
	this->consumer = consumer;
	return *this;
}

Pipeline& Pipeline::setTexture(Texture* texture, int unit) {
	if(unit == textures.size()) {
		textures.push_back(texture);
	}else if(unit < textures.size()) {
		textures[unit] = texture;
	}else {
		logger::error("Texture unit ", unit, " out of range [0, ", textures.size() + "]");
		throw std::out_of_range("Invalid texture unit!");
	}
	return *this;
}

void Pipeline::bind() {
	shader->bind();

	for(int unit = 0; unit < textures.size(); unit ++) {
		this->textures[unit]->bind(unit);
	}	

	consumer->bind();
}

void Pipeline::draw() {
	consumer->draw();
}

