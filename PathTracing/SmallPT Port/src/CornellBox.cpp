#include "CornellBox.h"
#include <stdio.h>

#define DEFAULT_MAX_DEPTH 1
#define DEFAULT_SAMPLES 1

inline float3 Vec() { return make_float3(0.0f, 0.0f, 0.0f); }
inline float3 Vec(double x, double y, double z) { return make_float3((float)x, (float)y, (float)z); }

CornellBox::CornellBox(unsigned width, unsigned height) : OptixScene(width, height), maxDepth(DEFAULT_MAX_DEPTH), samples(DEFAULT_SAMPLES) {
	printf("Initializing OptiX\n");
	context = Context::create();
	context->setEntryPointCount(1u);
	context->setRayTypeCount(1u);
	context->setStackSize(4096u);
	initScene();
}

CornellBox::~CornellBox() {
	context->destroy();
}

void CornellBox::setTarget(unsigned vbo) {	// THIS MUST BE CALLED
	Buffer buffer = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, vbo);
	buffer->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
	buffer->setSize(width, height);
	//->;
	context["output"]->set(buffer);
	printf("Compiling Context\n");
	context->validate();
	context->compile();
}

void CornellBox::render() {
	context->launch(0, width, height);
}

void CornellBox::initScene() {
	printf("Loading Geometry\n");
	const char* file = "PTX/Sphere.cu.ptx";
	sphere = context->createGeometry();
	sphere->setPrimitiveCount(1u);
	sphere->setBoundingBoxProgram(context->createProgramFromPTXFile(file, "bounds"));
	sphere->setIntersectionProgram(context->createProgramFromPTXFile(file, "intersect"));
	file = "PTX/Plane.cu.ptx";
	plane = context->createGeometry();
	plane->setPrimitiveCount(1u);
	plane->setBoundingBoxProgram(context->createProgramFromPTXFile(file, "bounds"));
	plane->setIntersectionProgram(context->createProgramFromPTXFile(file, "intersect"));

	printf("Loading Camera\n");
	file = "PTX/Camera.cu.ptx";
	context->setRayGenerationProgram(0, context->createProgramFromPTXFile(file, "trace"));
	context->setExceptionProgram(0, context->createProgramFromPTXFile(file, "exception"));
	context->setMissProgram(0, context->createProgramFromPTXFile(file, "miss"));

	printf("Loading Materials\n");
	file = "PTX/Materials.cu.ptx";
	Program diff = context->createProgramFromPTXFile(file, "diffuse");
	Program spec = context->createProgramFromPTXFile(file, "specular");
	Program refr = context->createProgramFromPTXFile(file, "refractive");

	printf("Creating Scene\n");
	GeometryGroup group = context->createGeometryGroup();
	group->setChildCount(9u);	// 9 Elements
	group->setChild(0, createPlane(Vec(0, 100, 100), Vec(1, 40.8, 81.6), Vec(1, 0, 0), Vec(), Vec(.75, .25, .25), diff));		// Left
	group->setChild(1, createPlane(Vec(0, 100, 100), Vec(99, 40.8, 81.6), Vec(-1, 0, 0), Vec(), Vec(.25, .25, .75), diff));	// Right
	group->setChild(2, createPlane(Vec(100, 100, 0), Vec(50, 40.8, 0), Vec(0, 0, 1), Vec(), Vec(.75, .75, .75), diff));			// Back
	group->setChild(3, createPlane(Vec(100, 100, 0), Vec(50, 40.8, 170), Vec(0, 0, -1), Vec(), Vec(), diff));			// Front
	group->setChild(4, createPlane(Vec(100, 0, 100), Vec(50, 0, 81.6), Vec(0, 1, 0), Vec(), Vec(.75, .75, .75), diff));			// Bottom
	group->setChild(5, createPlane(Vec(100, 0, 100), Vec(50, 81.6, 81.6), Vec(0, -1, 0), Vec(), Vec(.75, .75, .75), diff));	// Top
	group->setChild(6, createSphere(16.5f, Vec(27, 16.5, 47), Vec(), Vec(1, 1, 1)*0.999f, spec));			// Mirror
	group->setChild(7, createSphere(16.5f, Vec(73, 16.5, 78), Vec(), Vec(1, 1, 1)*0.999f, refr));			// Glass
	group->setChild(8, createSphere(600.0f, Vec(50, 681.6 - .27, 81.6), Vec(12, 12, 12), Vec(), diff));		// Light
	group->setAcceleration(context->createAcceleration("Bvh", "BvhSingle"));	// Maybe pointless...	
	context["objects"]->set(group);
	context["maxDepth"]->setInt(8);
	context["samples"]->setInt(1);
	context["camOrigin"]->setFloat(50.0f, 52.0f, 295.6f);
	context["camDirection"]->setFloat(normalize(make_float3(0.0f, -0.042612f, -1.0f)));	

/*	GeometryGroup group = context->createGeometryGroup();
	group->setChildCount(3u);	// 2 Elements
	group->setChild(0, createPlane(Vec(10, 0, 10), Vec(0, 0, 0), Vec(0, 1, 0), Vec(), Vec(.75, .25, .25), diff));	// Floor
	group->setChild(1, createSphere(3.0f, Vec(0, 3, 0), Vec(), Vec(.75, .75, .75), diff));							// Ball
	group->setChild(2, createSphere(1.0f, Vec(0, 8, 0), Vec(12, 12, 12), Vec(), diff));								// Light
	group->setAcceleration(context->createAcceleration("Bvh", "BvhSingle"));	// Maybe pointless...	
	context["objects"]->set(group);
	context["maxDepth"]->setInt(4);
	context["samples"]->setInt(2);
	context["camOrigin"]->setFloat(Vec(0, 2, -8));
	context["camDirection"]->setFloat(normalize(Vec(0, 3, 0) - Vec(0, 2, -8)));	*/
}

GeometryInstance CornellBox::createSphere(float r, float3 p, float3 e, float3 c, Program type) {
	Material m = context->createMaterial();
	m->setClosestHitProgram(0, type);
	m["f"]->setFloat(c);
	m["e"]->setFloat(e);
	GeometryInstance instance = context->createGeometryInstance();
	instance->setGeometry(sphere);
	instance->setMaterialCount(1);
	instance->setMaterial(0, m);
	instance["radius"]->setFloat(r);
	instance["position"]->setFloat(p);
	return instance;
}

GeometryInstance CornellBox::createPlane(float3 r, float3 p, float3 n, float3 e, float3 c, Program type) {
	Material m = context->createMaterial();
	m->setClosestHitProgram(0, type);
	m["f"]->setFloat(c);
	m["e"]->setFloat(e);
	GeometryInstance instance = context->createGeometryInstance();
	instance->setGeometry(plane);
	instance->setMaterialCount(1);
	instance->setMaterial(0, m);
	instance["Po"]->setFloat(p);
	instance["N"]->setFloat(n);
	instance["R"]->setFloat(r);
	return instance;
}