#pragma once

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include "Constants.h"
#include <list>

using namespace optix;
using namespace std;

/*
 *	This class stores information about an optix scene, including the camera, materials, geometries
 *	and several internal OptiX objects such as the context
 */

class Scene {
	vector<BasicLight> lights;
public:
	static void init(int* arc, char** argv);

	Context context;
	const unsigned width, height;

	Scene(unsigned width, unsigned height, float3 eye, float3 at, float3 up, float hfov);
	~Scene();

	Material createPhongMaterial(float3 Ks, float3 Ka, float3 Kd, float exp, float3 ref);
	GeometryGroup loadOBJ(const char* filename, Material material);
	GeometryInstance createParallelogram(float3 v1, float3 v2, float3 anchor, Material material);
	void addLight(float3 pos, float3 color);
//	Geometry createSphere();	//uses bounds and intersect programs from this file
	void render();
};