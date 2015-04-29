#pragma once

#define GLEW_STATIC
#include <gl\glew.h>
#include <GLFW\glfw3.h>
#include <list>

//Creates a shader object from glsl source
GLuint createShader(GLenum type, const char* src);

//Creates a shader object from a source file
GLuint loadShader(GLenum type, const char* filename);

//Creates a shader program from a list of shader objects
GLuint createProgram(std::list<GLuint> shaders);