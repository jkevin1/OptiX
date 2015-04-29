#include "TestScene.h"
#include <stdio.h>

#pragma warning( disable : 4996 )

TestScene::TestScene(unsigned width, unsigned height, int samples, int maxDepth) : OptixScene(width, height) {
	printf("Initializing Context\n");
	context = Context::create();
	context->setEntryPointCount(1u);
	context->setRayTypeCount(1u);
	context->setStackSize(4096u);

	printf("Allocating Buffers: %dx%d\n", width, height);
	Buffer accum = context->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT3, width, height);
	context["accum"]->setBuffer(accum);
	Buffer image = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_BYTE4, width, height);
	context["image"]->setBuffer(image);

	initScene();

	Buffer lightBuffer = context->createBuffer(RT_BUFFER_INPUT);
	lightBuffer->setFormat(RT_FORMAT_USER);
	lightBuffer->setElementSize(sizeof(PotentialLight));
	lightBuffer->setSize(lights.size());
	memcpy(lightBuffer->map(), &lights[0], sizeof(PotentialLight) * lights.size());
	lightBuffer->unmap();
	context["lights"]->setBuffer(lightBuffer);

	printf("Initializing Scene with %d samples and maximum depth %d\n", samples, maxDepth);
	this->samples = samples;
	this->maxDepth = maxDepth;
	this->frame = 0;
	context["samples"]->setInt(samples);
	context["maxDepth"]->setInt(maxDepth);
	context["frame"]->setInt(frame);

	printf("Compiling OptiX Kernel\n");
	context->validate();
	context->compile();
}

TestScene::~TestScene() {
	printf("Destroying Context\n");
	context->destroy();
}

void TestScene::render() {
	context["frame"]->setInt(++frame);
	context->launch(0, width, height);
}

void TestScene::update(double time) {
	if (!paused) {
		angle += float(time);
		matte["position"]->setFloat(4 * cosf(angle / 3), 2.0f, 4 * sinf(2 * angle / 3));
		refl["position"]->setFloat(5 * cosf(angle), -5.0f, 5 * sinf(angle));
		refr["position"]->setFloat(5 * cosf(angle + M_PIf), -5.0f, 5 * sinf(angle + M_PIf));
		frame = 0;
	}
}

void TestScene::saveScreenshot(const char* filename) {
	//printf("Saving Screenshot: %s\n", filename);
	Buffer buffer = context["image"]->getBuffer();
	unsigned width, height;
	FILE* file = fopen(filename, "w");
	buffer->getSize(width, height);
	fprintf(file, "P3\n%d %d\n255\n", width, height);
	uchar4* data = (uchar4*)buffer->map();
	for (unsigned y = 0; y < height; y++) {
		for (unsigned x = 0; x < width; x++) {
			uchar4 c = data[(height - y - 1u) * width + x];
			fprintf(file, "%d %d %d ", c.x, c.y, c.z);
		}
	}
	buffer->unmap();
	fclose(file);
}

void TestScene::setTarget(unsigned vbo) {
	printf("Setting target %d\n", vbo);
	Buffer buffer = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, vbo);
	buffer->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
	buffer->setSize(width, height);
	context["image"]->set(buffer);
	context->validate();
	context->compile();
}

void TestScene::keyReleased(int key) {
	if (key == 'R') frame = 0;
	if (key == 'P') paused = !paused;
}

void TestScene::mousePressed(int button) {
	if (button == 0) mouse = true;
}

void TestScene::mouseReleased(int button) {
	if (button == 0) mouse = false;
}

void TestScene::mouseMoved(double x, double y, double dx, double dy) {
	if (mouse) {
		yaw -= float(dx / 4);
		pitch -= float(dy / 4);
		updateCamera();
		frame = 0;
	}
}
void TestScene::initScene() {
	printf("Initializing Scene\n");
	const char* file = "PTX/PathTracer.cu.ptx";
	context->setRayGenerationProgram(0, context->createProgramFromPTXFile(file, "pathTrace"));
	context->setExceptionProgram(0, context->createProgramFromPTXFile(file, "exception"));
	context->setMissProgram(0, context->createProgramFromPTXFile(file, "miss"));
	mouse = paused = false;
	pitch = yaw = angle = 0.0f;
	updateCamera();
	
	printf("Loading Geometry\n");
	Geometry geometry = loadGeometry("PTX/Sphere.cu.ptx");

	Material diffuse = context->createMaterial();
	diffuse->setClosestHitProgram(0, context->createProgramFromPTXFile(file, "diffuse"));
	Material reflect = context->createMaterial();
	reflect->setClosestHitProgram(0, context->createProgramFromPTXFile(file, "reflect"));
	Material refract = context->createMaterial();
	refract->setClosestHitProgram(0, context->createProgramFromPTXFile(file, "refract"));

	GeometryInstance left = createObject(geometry, diffuse, make_float3(0, 0, 0), make_float3(0.5f, 0.1f, 0.1f));
	left["radius"]->setFloat(990.0f);
	left["position"]->setFloat(-1000.0f, 0.0f, 0.0f);
	GeometryInstance right = createObject(geometry, diffuse, make_float3(0, 0, 0), make_float3(0.1f, 0.1f, 0.5f));
	right["radius"]->setFloat(990.0f);
	right["position"]->setFloat(1000.0f, 0.0f, 0.0f);
	GeometryInstance back = createObject(geometry, diffuse, make_float3(0, 0, 0), make_float3(0.5f, 0.5f, 0.5f));
	back["radius"]->setFloat(990.0f);
	back["position"]->setFloat(0.0f, 0.0f, 1000.0f);
	GeometryInstance floor = createObject(geometry, diffuse, make_float3(0, 0, 0), make_float3(0.99f, 0.99f, 0.99f));
	floor["radius"]->setFloat(990.0f);
	floor["position"]->setFloat(0.0f, -1000.0f, 0.0f);

	matte = createObject(geometry, diffuse, make_float3(0, 0, 0), make_float3(0.5f, 0.5f, 0.5f));
	matte["radius"]->setFloat(3.0f);
	matte["position"]->setFloat(0.0f, 2.0f, 0.0f);
	refl = createObject(geometry, reflect, make_float3(0, 0, 0), make_float3(0.5f, 0.5f, 0.5f));
	refl["radius"]->setFloat(3.0f);
	refl["position"]->setFloat(5.0f, -5.0f, 5.0f);
	refr = createObject(geometry, refract, make_float3(0, 0, 0), make_float3(0.5f, 0.5f, 0.5f));
	refr["radius"]->setFloat(4.0f);
	refr["position"]->setFloat(-5.0f, -5.0f, 0.0f);

	GeometryInstance light = createObject(geometry, diffuse, make_float3(16, 16, 16), make_float3(0, 0, 0));
	light["radius"]->setFloat(2.0f);
	light["position"]->setFloat(0.0f, 10.0f, 0.0f);

	PotentialLight emitter;
	emitter.radius = 2.0f;
	emitter.position = make_float3(0, 8, 0);
	lights.push_back(emitter);
	GeometryGroup group = context->createGeometryGroup();
	group->setChildCount(8u);	// 8 Elements
	group->setChild(0, left);
	group->setChild(1, right);
	group->setChild(2, back);
	group->setChild(3, floor);
	group->setChild(4, matte);
	group->setChild(5, refl);
	group->setChild(6, refr);
	group->setChild(7, light);
	//group->setAcceleration(context->createAcceleration("Bvh", "BvhSingle"));	// Maybe pointless...	
	group->setAcceleration(context->createAcceleration("NoAccel", "NoAccel"));
	context["objects"]->set(group);
}

Geometry TestScene::loadGeometry(const char* ptx) {
	Geometry instance = context->createGeometry();
	instance->setPrimitiveCount(1u);
	instance->setBoundingBoxProgram(context->createProgramFromPTXFile(ptx, "bounds"));
	instance->setIntersectionProgram(context->createProgramFromPTXFile(ptx, "intersect"));
	return instance;
}

GeometryInstance TestScene::createObject(Geometry geom, Material mat, float3 emission, float3 color) {
	static int id = 0;
	GeometryInstance instance = context->createGeometryInstance();
	instance->setGeometry(geom);
	instance->setMaterialCount(1);
	instance->setMaterial(0, mat);
	instance["emission"]->setFloat(emission);
	instance["color"]->setFloat(color);
	instance["gID"]->setInt(++id);
	return instance;
}

void TestScene::updateCamera() {
	float3 eye = make_float3(0.0f, 0.0f, -30.0f);
	float3 lookdir = normalize(make_float3(30 * sinf(M_PIf * yaw / 180.0f), 30 * sinf(M_PIf * pitch / 180.0f), 0) - eye);
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
}