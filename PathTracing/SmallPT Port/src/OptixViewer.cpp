#include "OptixViewer.h"
#define GLEW_STATIC
#include <gl\glew.h>
#include <GLFW\glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>

static void glfwErrorCallback(int error, const char* description) {
	fprintf(stderr, "GLFW error: %s\n", description);
}

OptixViewer::OptixViewer(OptixScene* scene, const char* title, bool vsync) : scene(scene) {
	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}
	/*
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
	*/
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = glfwCreateWindow(scene->width, scene->height, title, nullptr, nullptr);
	if (!window) {
		fprintf(stderr, "Failed to create window\n");
		exit(EXIT_FAILURE);
	}

	glfwSetWindowUserPointer(window, scene);
	// TODO: callbacks

	
}


OptixViewer::~OptixViewer() {
	glfwTerminate();
}

void renderThread(GLFWwindow* window) {
	OptixScene* scene = (OptixScene*)glfwGetWindowUserPointer(window);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		exit(EXIT_FAILURE);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, scene->width, scene->height);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	unsigned tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	unsigned vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * scene->width * scene->height, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	scene->setTarget(vbo);

	while (!glfwWindowShouldClose(window)) {
		//TODO update
		scene->render();

		glBindTexture(GL_TEXTURE_2D, tex);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vbo);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, scene->width, scene->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		glEnable(GL_TEXTURE_2D);
		float u = 0.5f / scene->width;
		float v = 0.5f / scene->height;
		glBegin(GL_QUADS);
		glTexCoord2f(u, v);
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, v);
		glVertex2f(1.0f, 0.0f);
		glTexCoord2f(1.0f - u, 1.0f - v);
		glVertex2f(1.0f, 1.0f);
		glTexCoord2f(u, 1.0f - v);
		glVertex2f(0.0f, 1.0f);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		glfwSwapBuffers(window);
		//glfwPollEvents();
	}
}

void OptixViewer::run() {
	std::thread renderer(renderThread, window);

	while (!glfwWindowShouldClose(window)) {
		glfwWaitEvents();
	}

	renderer.join();
}