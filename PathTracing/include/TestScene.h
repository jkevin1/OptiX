#pragma once

#include "OptixScene.h"
#include "SphereLight.h"
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

class TestScene : OptixScene {
public:
	TestScene(unsigned width, unsigned height, int samples, int maxDepth);
	virtual ~TestScene();

	virtual void render();
	virtual void saveScreenshot(const char* filename, float iterations = 1.0f);

private:
	Context context;
	int frame, samples, maxDepth;
	std::vector<PotentialLight> lights;
	void initScene();
	Geometry loadGeometry(const char* ptx);
	GeometryInstance createObject(Geometry geom, Material mat, float3 emission, float3 color);
};