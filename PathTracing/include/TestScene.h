#pragma once

#include "OptixScene.h"
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

class TestScene : OptixScene {
public:
	TestScene(unsigned width, unsigned height);
	virtual ~TestScene();

	virtual void render();
	virtual void saveScreenshot(const char* filename);

	void setNumSamples(int samples);
	void setMaxDepth(int depth);
private:
	Context context;
	void initScene();
	Geometry loadGeometry(const char* ptx);
	GeometryInstance createObject(Geometry geom, Material mat, float3 emission, float3 color);
};