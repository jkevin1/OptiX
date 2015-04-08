#include "TestScene.h"
#include <stdio.h>

#pragma warning( disable : 4996 )

TestScene::TestScene(unsigned width, unsigned height) : OptixScene(width, height) {
	printf("Initializing Context\n");
	context = Context::create();
	context->setEntryPointCount(1u);
	context->setRayTypeCount(1u);
	context->setStackSize(4096u);
	printf("Allocating Buffer: %dx%d\n", width, height);
	Buffer image = context->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT4, width, height);
	context["image"]->setBuffer(image);
	initScene();
	Buffer lightBuffer = context->createBuffer(RT_BUFFER_INPUT);
	lightBuffer->setFormat(RT_FORMAT_USER);
	lightBuffer->setElementSize(sizeof(SphereLight));
	lightBuffer->setSize(lights.size());
	memcpy(lightBuffer->map(), &lights[0], sizeof(SphereLight) * lights.size());
	lightBuffer->unmap();
	context["lights"]->setBuffer(lightBuffer);
	printf("Compiling OptiX Kernel\n");
	context->validate();
	context->compile();
}

TestScene::~TestScene() {
	printf("Destroying Context\n");
	context->destroy();
}

void TestScene::render() {
	context->launch(0, width, height);
}

inline unsigned char toInt(float x) {
	return int((x < 0 ? 0 : (x > 1 ? 1 : x)) * 255 + .5f);
}

void TestScene::saveScreenshot(const char* filename, float iterations) {
	//printf("Saving Screenshot: %s\n", filename);
	Buffer buffer = context["image"]->getBuffer();
	unsigned width, height;
	FILE* file = fopen(filename, "w");
	buffer->getSize(width, height);
	fprintf(file, "P3\n%d %d\n255\n", width, height);
	float4* data = (float4*)buffer->map();
	for (unsigned y = 0; y < height; y++) {
		for (unsigned x = 0; x < width; x++) {
			float4 c = data[(height - y - 1u) * width + x];
			c /= iterations;
			fprintf(file, "%d %d %d ", toInt(c.x), toInt(c.y), toInt(c.z));
		}
	}
	buffer->unmap();
	fclose(file);
}

void TestScene::initScene() {
	printf("Initializing Scene\n");
	const char* file = "PTX/PathTracer.cu.ptx";
	context->setRayGenerationProgram(0, context->createProgramFromPTXFile(file, "pathTrace"));
	context->setExceptionProgram(0, context->createProgramFromPTXFile(file, "exception"));
	context->setMissProgram(0, context->createProgramFromPTXFile(file, "miss"));
	float3 eye = make_float3(0.0f, 0.0f, -30.0f);
	float3 lookdir = normalize(make_float3(0, 0, 0) - eye);
	float3 camera_u = cross(lookdir, make_float3(0, 1, 0));
	float3 camera_v = cross(camera_u, lookdir);
	float ulen = tanf(50.0f / 2.0f * M_PIf / 180.0f);
	camera_u = normalize(camera_u);
	camera_u *= ulen;
	float aspect_ratio = float(width) / float(height);
	float vlen = ulen / aspect_ratio;
	camera_v = normalize(camera_v);
	camera_v *= vlen;
	context["eye"]->setFloat(eye.x, eye.y, eye.z);
	context["U"]->setFloat(camera_u.x, camera_u.y, camera_u.z);
	context["V"]->setFloat(camera_v.x, camera_v.y, camera_v.z);
	context["W"]->setFloat(lookdir.x, lookdir.y, lookdir.z);
	
	printf("Loading Geometry\n");
	Geometry geometry = loadGeometry("PTX/Sphere.cu.ptx");
	Material material = context->createMaterial();
	material->setClosestHitProgram(0, context->createProgramFromPTXFile(file, "diffuse"));
	GeometryInstance left = createObject(geometry, material, make_float3(0, 0, 0), make_float3(0.9f, 0.1f, 0.1f));
	left["radius"]->setFloat(990.0f);
	left["position"]->setFloat(-1000.0f, 0.0f, 0.0f);
	GeometryInstance right = createObject(geometry, material, make_float3(0, 0, 0), make_float3(0.1f, 0.1f, 0.9f));
	right["radius"]->setFloat(990.0f);
	right["position"]->setFloat(1000.0f, 0.0f, 0.0f);
	GeometryInstance back = createObject(geometry, material, make_float3(0, 0, 0), make_float3(0.8f, 0.8f, 0.8f));
	back["radius"]->setFloat(990.0f);
	back["position"]->setFloat(0.0f, 0.0f, 1000.0f);
	GeometryInstance center = createObject(geometry, material, make_float3(0, 0, 0), make_float3(0.8f, 0.8f, 0.8f));
	center["radius"]->setFloat(5.0f);
	center["position"]->setFloat(0.0f, -2.0f, 0.0f);
	GeometryInstance light = createObject(geometry, material, make_float3(16, 16, 16), make_float3(0, 0, 0));
	light["radius"]->setFloat(2.0f);
	light["position"]->setFloat(0.0f, 8.0f, 0.0f);
	SphereLight emitter;
	emitter.emission = make_float3(16, 16, 16);
	emitter.radius = 2.0f;
	emitter.position = make_float3(0, 8, 0);
	lights.push_back(emitter);
	GeometryGroup group = context->createGeometryGroup();
	group->setChildCount(5u);	// 4 Elements
	group->setChild(0, left);
	group->setChild(1, right);
	group->setChild(2, back);
	group->setChild(3, center);
	group->setChild(4, light);
	group->setAcceleration(context->createAcceleration("Bvh", "BvhSingle"));	// Maybe pointless...	
	context["objects"]->set(group);
	setNumSamples(2048);
	setMaxDepth(4);
}

void TestScene::setNumSamples(int samples) {
	context["NUM_SAMPLES"]->setInt(samples);
}

void TestScene::setMaxDepth(int depth) {
	context["MAX_DEPTH"]->setInt(depth);
}


Geometry TestScene::loadGeometry(const char* ptx) {
	Geometry instance = context->createGeometry();
	instance->setPrimitiveCount(1u);
	instance->setBoundingBoxProgram(context->createProgramFromPTXFile(ptx, "bounds"));
	instance->setIntersectionProgram(context->createProgramFromPTXFile(ptx, "intersect"));
	return instance;
}

GeometryInstance TestScene::createObject(Geometry geom, Material mat, float3 emission, float3 color) {
	GeometryInstance instance = context->createGeometryInstance();
	instance->setGeometry(geom);
	instance->setMaterialCount(1);
	instance->setMaterial(0, mat);
	instance["emission"]->setFloat(emission);
	instance["color"]->setFloat(color);
	return instance;
}

void TestScene::setFrame(int frame) {
	context["frame"]->setInt(frame);
}