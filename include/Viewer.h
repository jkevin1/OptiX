#pragma once

struct GLFWwindow;

class Viewer {
	GLFWwindow* window;
	unsigned vbo, tex;
public:
	const unsigned width, height;

	Viewer(const char* title, unsigned width, unsigned height, bool vsync = false);
	~Viewer();

	bool isOpen();

	unsigned getVBO();

	void render();
};