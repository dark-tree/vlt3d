#pragma once

#include <logger.hpp>
#include <renderer/shader.hpp>
#include <renderer/texture.hpp>
#include <renderer/vertex.hpp>
#include <include/external.hpp>

class Pipeline {
	
	public:

		ShaderProgram* shader;
		VertexConsumer* consumer;
		std::vector<Texture*> textures;

	public:
		
		Pipeline& setShader(ShaderProgram* shader);

		Pipeline& setConsumer(VertexConsumer* consumer);

		Pipeline& setTexture(Texture* texture, int unit);

		void bind();

		void draw();
		
};


