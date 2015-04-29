#include "Shaders.h"
#include <stdio.h>

static char* readFile(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (!file) {
		perror(filename);
		return nullptr;
	}

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* src = new char[size + 1];
	fread(src, 1, size, file);
	src[size] = '\0';
	fclose(file);
	return src;
}

GLuint createShader(GLenum type, const char* src) {
	GLuint id = glCreateShader(type);
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	GLint compiled;
	glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
	if (compiled) return id;

	GLsizei size;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &size);
	char* error = new char[size];
	glGetShaderInfoLog(id, size, 0, error);
	fprintf(stderr, "Shader Error:\n%s", error);
	delete[] error;
	glDeleteShader(id);
	return 0;
}

GLuint loadShader(GLenum type, const char* filename) {
	char* src = readFile(filename);
	if (!src) return 0;
	GLuint id = createShader(type, src);
	delete[] src;
	return id;
}

GLuint createProgram(std::list<GLuint> shaders) {
	GLuint id = glCreateProgram();
	for (auto iterator = shaders.begin(); iterator != shaders.end(); ++iterator)
		glAttachShader(id, *iterator);
	glLinkProgram(id);

	GLint linked;
	glGetProgramiv(id, GL_LINK_STATUS, &linked);
	if (linked) return id;

	GLsizei size;
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &size);
	char* error = new char[size];
	glGetProgramInfoLog(id, size, 0, error);
	fprintf(stderr, "Program Error:\n%s", error);
	delete[] error;
	glDeleteProgram(id);
	return 0;
}