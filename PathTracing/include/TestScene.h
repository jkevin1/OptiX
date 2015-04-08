#pragma once

#include "OptixScene.h"
#include "SphereLight.h"
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

class TestScene : OptixScene {
public:
	TestScene(unsigned width, unsigned height);
	virtual ~TestScene();

	virtual void render();
	virtual void saveScreenshot(const char* filename, float iterations = 1.0f);

	void setNumSamples(int samples);
	void setMaxDepth(int depth);
	void setFrame(int frame);
private:
	Context context;
	std::vector<SphereLight> lights;
	void initScene();
	Geometry loadGeometry(const char* ptx);
	GeometryInstance createObject(Geometry geom, Material mat, float3 emission, float3 color);
};