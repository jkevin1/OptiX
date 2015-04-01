#pragma once

#include "OptixScene.h"
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

class CornellBox : public OptixScene {
public:
	CornellBox(unsigned width, unsigned height);
	virtual ~CornellBox();
	
	virtual void setTarget(unsigned vbo);
	virtual void render();
private:
	int samples, maxDepth;
	Context context;
	Geometry sphere, plane;
	void initScene();
	GeometryInstance createSphere(float r, float3 p, float3 e, float3 c, Program type);
	GeometryInstance createPlane(float3 r, float3 p, float3 n, float3 e, float3 c, Program type);
};