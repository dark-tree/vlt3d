#pragma once

#include <logger.hpp>
#include <renderer/uniform.hpp>
#include <include/external.hpp>

class ShaderProgram {

	private:
		GLuint program;

	public:
		ShaderProgram(GLuint);
		~ShaderProgram();

		GLuint id();
		Uniform location(const char* name);
		GLuint buffer(GLenum interface, const char* name);

		template<typename T>
		void set(const char* name, const T& value) {
			location(name).set(value);
		}

		void bind();
};

class ShaderProgramBuilder {

	private:
		std::vector<GLuint> shaders;
		std::unordered_map<std::string, std::string> constants;
		GLint id;

		std::string parseSource( const std::string& source, std::string& path );

	public:
		void setConstant( const std::string& name, const std::string& value = "" );
		bool compileSource( std::string, std::string, GLenum );
		bool compileFile( std::string, std::string, GLenum );
		bool link();

		ShaderProgram* build();
};

