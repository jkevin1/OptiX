#pragma once

#include <list>
#include "OptiX.h"
#include "Common.h"

/*
 *	This class stores information about an optix scene, including the camera, materials, geometries
 *	and several internal OptiX objects such as the context
 */

class Scene {
	std::vector<BasicLight> lights;
	std::vector<GeometryInstance> objects;
public:
	const unsigned width, height;
	Context context;

	Scene(unsigned width, unsigned height, unsigned vbo);
	~Scene();

	void setCamera(float3 eye, float3 at, float3 up, float hfov);

	void addLight(float3 pos, float3 color);

	GeometryInstance& addObject(Geometry type, Material material);
	GeometryInstance& addObject(GeometryInstance object);
	GeometryInstance& addObject(GeometryGroup object);

	void compile();
	void render();
};