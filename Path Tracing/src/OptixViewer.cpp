#include "OptixViewer.h"
#include "Shaders.h"
#include <stdlib.h>
#include <stdio.h>

static void glfwErrorCallback(int error, const char* description) {
	fprintf(stderr, "GLFW error: %s\n", description);
}

static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	OptixScene* scene = (OptixScene*)glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS) {
		scene->keyPressed(key);
	} else if (action == GLFW_RELEASE) {
		scene->keyReleased(key);
	}
}

static void glfwMouseCallback(GLFWwindow* window, int button, int action, int mods) {
	OptixScene* scene = (OptixScene*)glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS) {
		scene->mousePressed(button);
	}
	else if (action == GLFW_RELEASE) {
		scene->mouseReleased(button);
	}
}

static void glfwCursorCallback(GLFWwindow* window, double x, double y) {
	static double x0 = x;
	static double y0 = y;
	OptixScene* scene = (OptixScene*)glfwGetWindowUserPointer(window);
	scene->mouseMoved(x, y, x - x0, y - y0);
	x0 = x; y0 = y;
}

OptixViewer::OptixViewer(OptixScene* scene, const char* title, bool vsync) : scene(scene) {
	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
	
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = glfwCreateWindow(scene->width, scene->height, title, nullptr, nullptr);
	if (!window) {
		fprintf(stderr, "Failed to create window\n");
		exit(EXIT_FAILURE);
	}

	glfwSetWindowUserPointer(window, scene);
	glfwSetKeyCallback(window, glfwKeyCallback);
	glfwSetMouseButtonCallback(window, glfwMouseCallback);
	glfwSetCursorPosCallback(window, glfwCursorCallback);
	// TODO: callbacks
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		exit(EXIT_FAILURE);
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glViewport(0, 0, scene->width, scene->height);

	static const GLfloat vertices[] = {
		-1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
	};

	glGenBuffers(1, &ptr);
	glBindBuffer(GL_ARRAY_BUFFER, ptr);
	glBufferData(GL_ARRAY_BUFFER, 4 * scene->width * scene->height, 0, GL_STREAM_DRAW);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, ptr);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	GLuint vert = loadShader(GL_VERTEX_SHADER, "res/vert.glsl");
	GLuint frag = loadShader(GL_FRAGMENT_SHADER, "res/frag.glsl");
	GLuint prog = createProgram({ vert, frag });
	glUseProgram(prog);
	glUniform1i(glGetUniformLocation(prog, "image"), 0);
	glUniform1f(glGetUniformLocation(prog, "invWidth"), 1.0f / scene->width);
	glUniform1f(glGetUniformLocation(prog, "invHeight"), 1.0f / scene->height);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	scene->setTarget(ptr);
}

OptixViewer::~OptixViewer() {
	glfwTerminate();
}

void OptixViewer::run() {
	double prev = glfwGetTime(), curr = glfwGetTime();
	double time = 0.0;
	int frames = 0;

	while (!glfwWindowShouldClose(window)) {
		scene->update(curr - prev);
		scene->render();

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scene->width, scene->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glfwSwapBuffers(window);
		glfwPollEvents();

		prev = curr;
		curr = glfwGetTime();
		time += curr - prev;
		frames++;
		if (time >= 1.0) {
			printf("Average Frame Time: %fms (%dfps)\n", 1.0 / frames, frames);
			time -= 1.0;
			frames = 0;
		}
	}
}