#pragma once

#include "OptixScene.h"
#include "SphereLight.h"
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

class TestScene : public OptixScene {
public:
	TestScene(unsigned width, unsigned height, int samples, int maxDepth);
	virtual ~TestScene();

	virtual void render();
	virtual void update(double time);
	virtual void saveScreenshot(const char* filename);
	virtual void setTarget(unsigned vbo);

	virtual void keyReleased(int key);
	virtual void mousePressed(int button);
	virtual void mouseReleased(int button);
	virtual void mouseMoved(double x, double y, double dx, double dy);
private:
	Context context;
	int frame, samples, maxDepth;
	bool mouse, paused;
	float pitch, yaw, angle;
	GeometryInstance matte, refl, refr;
	std::vector<PotentialLight> lights;
	void initScene();
	Geometry loadGeometry(const char* ptx);
	GeometryInstance createObject(Geometry geom, Material mat, float3 emission, float3 color);
	void updateCamera();
};