
#include "shader.hpp"

ShaderProgram::ShaderProgram(GLuint id) {
    this->program = id;
}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(this->program);
}

Uniform ShaderProgram::location( const char* name ) {
    return Uniform( glGetUniformLocation(this->program, name) );
}

GLuint ShaderProgram::buffer( GLenum interface, const char* name ) {
	return glGetProgramResourceIndex(this->program, interface, name);
}

GLuint ShaderProgram::id() {
    return this->program;
}

void ShaderProgram::bind() {
    glUseProgram(this->program);
}

void ShaderProgramBuilder::setConstant( const std::string& name, const std::string& value ) {
	this->constants[name] = value;
}

std::string ShaderProgramBuilder::parseSource( const std::string& input, std::string& path ) {
	
	std::string command_constant = "#constant ";
	std::string command_require = "#require ";

	std::stringstream sstr(input);
	std::string line, source;

	while( std::getline(sstr, line) ) {

		if( !line.compare(0, command_constant.size(), command_constant) ) {
			std::string constant = line.substr( command_constant.size() );

			try{
				std::string value = this->constants.at(constant);
			
				source += "#define " + constant + " " + value + "\n";
			}catch(std::out_of_range& err) {
				logger::warn( "Unknown constant '", constant, "' requested in shader '", path, "'!" );
			}

			continue;
		}

		if( !line.compare(0, command_require.size(), command_require) ) {
			std::string required = line.substr( command_require.size() );

			std::ifstream ifile(path + required, std::ios::in);

			if( ifile.is_open() ) {
				std::stringstream fsstr;
				fsstr << ifile.rdbuf();
				ifile.close();

				source += parseSource(fsstr.str(), path) + "\n";
			}

			continue;
		}
			
		source += line + "\n";
	}

	return source;

}

bool ShaderProgramBuilder::compileSource( std::string source, std::string path, GLenum type ) {
    GLint shader = glCreateShader(type);
    GLint result = GL_FALSE;
	int log_length;

	source = parseSource(source, path);

    // compile shader
	const char* cstr = source.c_str();
	glShaderSource(shader, 1, &cstr, NULL);
	glCompileShader(shader);

	// check compilation status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

    // print status
	if( !result && log_length > 0 ) {
		std::vector<char> log(log_length + 1);
		glGetShaderInfoLog(shader, log_length, NULL, &log[0]);
        logger::error( "Failed to compile OpenGL shader: '", path, "'!" );         
		printf("%s\n", &log[0]);
		return false;
	}
        
	this->shaders.push_back( shader );

	return true;
}

bool ShaderProgramBuilder::compileFile( std::string path, std::string filename, GLenum type ) {

	std::string fullpath = path + filename;
    std::string source;
	std::ifstream ifile(fullpath, std::ios::in);

	if( ifile.is_open() ) {
		std::stringstream sstr;
		sstr << ifile.rdbuf();
		source = sstr.str();
		ifile.close();

        // compile shader source
        return this->compileSource( source, path, type );
	}else{
		logger::error( "Failed to open OpenGL shader file: '", path, "'!" );
        return false;
	}

}

bool ShaderProgramBuilder::link() {

    if( this->shaders.size() < 2 ) {
        logger::error( "Failed to link OpenGL shader program, expected at least two shaders!" );
		return false;
    }

	GLuint program = glCreateProgram();
    GLint result = GL_FALSE;
	int log_length;

    for( GLint shader : this->shaders ) {
        glAttachShader(program, shader);
    }

    glLinkProgram(program);

    // check the program
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

	if ( !result && log_length > 0 ){
		std::vector<char> log(log_length + 1);
		glGetProgramInfoLog(program, log_length, NULL, &log[0]);
        logger::error( "Failed to link OpenGL shader program!" );         
		printf("%s\n", &log[0]);
        return false;
	}

    for( GLint shader : this->shaders ) {
        glDetachShader(program, shader);
	    glDeleteShader(shader);
    }

	this->id = program;

    return true;
}

ShaderProgram* ShaderProgramBuilder::build() {
    return new ShaderProgram(this->id);
}

