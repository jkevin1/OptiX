#pragma once

struct GLFWwindow;

class Viewer {
	const unsigned width, height;
	GLFWwindow* window;
	unsigned vbo, tex;
public:
	Viewer(const char* title, unsigned width, unsigned height, bool vsync = false);
	~Viewer();

	int isClosed();

	unsigned getBuffer();

	void render();

	void setTitle(const char* title);
};