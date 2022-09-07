#pragma once

#include "shader.hpp"
#include "light.hpp"
#include "buffer/storage.hpp"
#include <include/external.hpp>

class Context {

	private:

		StorageBuffer<Light> lights;
		GLuint lights_buffer_base = 0;

	public:

		Context() : lights(0) {
			
		}

		ShaderProgram* loadShader(const std::string& name) {
			ShaderProgramBuilder builder;

			// provide constants
			//builder.setConstant("USE_FALLBACK_STORAGE", "");
			builder.setConstant("MAX_BUFFER_LENGTH", "128");

			bool vertex = builder.compileFile( "assets/" + name + "/", "vertex.glsl", GL_VERTEX_SHADER );
			bool fragment = builder.compileFile( "assets/" + name + "/", "fragment.glsl", GL_FRAGMENT_SHADER );

			if( vertex && fragment ) {
		
				if( builder.link() ) {
					logger::info( "Loaded OpenGL shader program: '", name, "'" );
					return builder.build();
				}

			}

			throw std::runtime_error("OpenGL shader program failed to load!");
		}

		Light& addLight( LightType type ) {
			lights.push(Light(type));
			return lights.get(lights.count() - 1);
		}

		Light& getLight( int index ) {
			return lights.get(index);
		}

		Light& peekLight( int index ) {
			return lights.peek(index);
		}

		void removeLight( int index ) {
			lights.remove(index);
		}

		int lightCount() {
			return lights.count();
		}

		void submit() {
			lights.submit();
		}

};

