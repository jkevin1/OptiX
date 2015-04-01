#pragma once

#include "OptixScene.h"
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

class TestScene : public OptixScene {
public:
	TestScene(unsigned width, unsigned height);
	virtual ~TestScene();

	virtual void setTarget(unsigned vbo);
	virtual void render();
private:
	Context context;
};