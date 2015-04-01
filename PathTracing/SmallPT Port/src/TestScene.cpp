#include "TestScene.h"
#include <stdio.h>

TestScene::TestScene(unsigned width, unsigned height) : OptixScene(width, height) {
	printf("Initializing OptiX\n");
	context = Context::create();
	context->setEntryPointCount(1u);
	context->setRayTypeCount(1u);
	context->setStackSize(4096u);

	printf("Loading Scene\n");
	std::string camera_ptx = "camera.cu.ptx";
	context->setRayGenerationProgram(0, context->createProgramFromPTXFile(camera_ptx, "raygen"));
	context->setExceptionProgram(0, context->createProgramFromPTXFile(camera_ptx, "exception"));
	context->setMissProgram(0, context->createProgramFromPTXFile(camera_ptx, "miss"));
	context["background"]->setFloat(0.5f, 1.0f, 0.5f);

	printf("Creating Scene\n");
	GeometryGroup group = context->createGeometryGroup();
	group->setChildCount(0u);	// 9 Spheres
	group->setAcceleration(context->createAcceleration("NoAccel", "NoAccel"));	
	context["objects"]->set(group);
}

TestScene::~TestScene() {
	context->destroy();
}

void TestScene::setTarget(unsigned vbo) {	// THIS MUST BE CALLED
	Buffer buffer = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, vbo);
	buffer->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
	buffer->setSize(width, height);
	context["output_buffer"]->set(buffer);
	printf("Compiling Context\n");
	context->validate();
	context->compile();
}

void TestScene::render() {
	printf("Rendering\n");
	context->launch(0, width, height);
	printf("Done\n");
}