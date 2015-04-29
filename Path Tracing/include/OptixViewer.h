#pragma once

#include "OptixScene.h"

struct GLFWwindow;

class OptixViewer {
	OptixScene* scene;
	GLFWwindow* window;
	unsigned vao, vbo;
	unsigned ptr, tex;
public:
	OptixViewer(OptixScene* scene, const char* title, bool vsync = false);
	~OptixViewer();
	void run();
};