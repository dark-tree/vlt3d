
const GLfloat data[] = {
	-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
	 1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
	 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
};

GLuint shader(GLenum type, const char* source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	return shader;
}

void vertexAttribute(int index, int count, int stride, long offset) {
	glVertexAttribPointer(index, count, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) (offset * sizeof(float)));
	glEnableVertexAttribArray(index);
}

void init(){
	glClearColor(0.0, 0.0, 0.0, 1.0);

	GLuint vert = shader( GL_VERTEX_SHADER,
		"#version 330 core\n"
		"layout (location = 0) in vec2 pos;\n"
		"layout (location = 1) in vec3 rgb;\n"
		"out vec3 color;\n"
		"void main(){gl_Position = vec4(pos.x, pos.y, -1.0, 1.0); color = rgb;\n}"
	);

	GLuint frag = shader(GL_FRAGMENT_SHADER,
		"#version 330 core\n"
		"in vec3 color;\n"
		"out vec4 frag;\n"
		"void main(){frag = vec4(color, 1);}"
	);

	GLuint program = glCreateProgram();
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	glLinkProgram(program);

	glDeleteShader(vert);
	glDeleteShader(frag);

	glUseProgram(program);

	// create and bind VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// create and fill VBO
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	// configure VAO
	vertexAttribute(0, 2, 5, 0); // vec2: pos
	vertexAttribute(1, 3, 5, 2); // vec3: rgb
}

void draw(){
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

