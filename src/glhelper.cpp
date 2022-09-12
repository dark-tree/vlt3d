
#include "glhelper.hpp"

bool GLHelper::init(int width, int height, const char* name) {

	if (!Input::open(width, height, name)) {
		return false;
	}

	gladLoadGL();

	auto& renderer = RenderSystem::instance();

	// specify window size to OpenGL
	glViewport(0, 0, width, height);

	// Time to enter the third dimension!
	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_MULTISAMPLE);

	glEnable(GL_CULL_FACE);

	// allow to override color if the depth is equal
	renderer.setDepthFunc(GL_LEQUAL);
	renderer.depthTest(true);

	// enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	srand(time(0));

	// remove frame cap - for performance testing
	// it's not 100% relible on all systems/drivers
	winxSetVsync(WINX_VSYNC_DISABLED);

	// stb image config
	stbi_set_flip_vertically_on_load(true);
	stbi_flip_vertically_on_write(true);

	return true;

}

void GLHelper::frame() {

	// Swap buffers
	winxSwapBuffers();

	// poll for GLFW window events
	winxPollEvents();

	// check for OpenGL error
	getError();

	// clear the screen and depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);

}

byte* GLHelper::capture( int* width, int* height ) {

	// get window size
	int viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );

	*width = viewport[2], *height = viewport[3];

	// create RGB image buffer
	byte* pixels = new byte[3 * *width * *height];

	// read from framebuffer
	glReadPixels(0, 0, *width, *height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	return pixels;
}

void GLHelper::screenshot( const std::string& path ) {

	int width, height;
	byte* pixels = GLHelper::capture( &width, &height );

	// write buffer to file
	if( stbi_write_png( path.c_str(), width, height, 3 /*RGB*/, pixels, 0 ) == 0 ) {
		logger::error("Failed to write screenshot to: '", path, "'!");
	}else{
		logger::info("Written screenshot to: '", path, "'");
	}

	// free buffer
	delete[] pixels;

}

void GLHelper::getError( const char* origin ) {
	GLenum err = glGetError();

	std::string name;

	switch( err ) {
		case GL_NO_ERROR: name = "GL_NO_ERROR"; break;
		case GL_INVALID_ENUM: name = "GL_INVALID_ENUM"; break;
		case GL_INVALID_VALUE: name = "GL_INVALID_VALUE"; break;
		case GL_INVALID_OPERATION: name = "GL_INVALID_OPERATION"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: name = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
		case GL_OUT_OF_MEMORY: name = "GL_OUT_OF_MEMORY"; break;
		case GL_STACK_UNDERFLOW: name = "GL_STACK_UNDERFLOW"; break;
		case GL_STACK_OVERFLOW: name = "GL_STACK_OVERFLOW"; break;
		default: name = "UNKNOWN";
	}

	std::string location = origin == nullptr ? "" : std::string(", at: '") + origin + std::string("'");

	if( err != GL_NO_ERROR ) {
		logger::warn( "OpenGL Error: ", err, ", ", name, location, "!" );
	}

	std::cout << std::flush;
}

void GLHelper::vertexAttribute( GLint index, GLint length, GLenum type, GLsizei stride, GLsizei offset, GLsizei size, GLboolean normalize, GLuint divisor ) {
	glVertexAttribPointer(index, length, type, normalize, stride * size, (GLvoid*) (long) (offset * size));
	glEnableVertexAttribArray(index);
	glVertexAttribDivisor(index, divisor);
}

ShaderProgram* GLHelper::loadShaderProgram( std::string name ) {
	ShaderProgramBuilder builder;
	builder.setConstant("MAX_LIGHT_COUNT", "64");

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

