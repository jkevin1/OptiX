#include "Scene.h"
#include <sutil.h>
#include <ObjLoader.h>

#define CAMERA_PTX "camera.cu.ptx"
#define PHONG_PTX "phong.cu.ptx"
#define PARALLELOGRAM_PTX "parallelogram.cu.ptx"

void Scene::init(int* argc, char** argv) {
	RT_CHECK_ERROR_NO_CONTEXT(sutilInitGlut(argc, argv));	
}

Scene::Scene(unsigned width, unsigned height, float3 eye, float3 at, float3 up, float hfov) : context(Context::create()), width(width), height(height) {
	//Initialize the context
	context->setEntryPointCount(1);
	context->setRayTypeCount(2);	//radiance and shadow
	context->setStackSize(2048);	//arbitrary

	//TODO: SET CONTEXT CONSTANTS
	//context["name"]->setX(y);

	//Create the output buffer
	Variable output_buffer = context["output_buffer"];
	Buffer buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_UNSIGNED_BYTE4, width, height);
	output_buffer->set(buffer);
	
	//Load ray generation, exception, and miss programs
	context->setRayGenerationProgram(0, context->createProgramFromPTXFile(CAMERA_PTX, "raygen"));
	context->setExceptionProgram(0, context->createProgramFromPTXFile(CAMERA_PTX, "exception"));
	context["exception_color"]->setFloat(1.0f, 1.0f, 0.0f);
	context->setMissProgram(0, context->createProgramFromPTXFile(CAMERA_PTX, "miss"));
	context["background"]->setFloat(0.0f, 1.0f, 1.0f);

	//Set up the camera
	float3 lookdir = normalize(at - eye);
	float3 camera_u = cross(lookdir, up);
	float3 camera_v = cross(camera_u, lookdir);
	float ulen = tanf(hfov / 2.0f * M_PIf / 180.0f);
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

Scene::~Scene() {
	//Release resources
	context->destroy();
}

Material Scene::createPhongMaterial(float3 Ks, float3 Ka, float3 Kd, float exp, float3 ref) {
	//Create a new material
	Material material = context->createMaterial();

	//Register radiance program for the radiance ray type
    material->setClosestHitProgram(RADIANCE_RAY, context->createProgramFromPTXFile(PHONG_PTX, "radiance"));

	//Register shadow program for the shadow ray type
    material->setAnyHitProgram(SHADOW_RAY, context->createProgramFromPTXFile(PHONG_PTX, "shadow"));

    //Set the material properties
    material["Ks"]->setFloat(Ks.x, Ks.y, Ks.z);
    material["Ka"]->setFloat(Ka.x, Ka.y, Ka.z);
    material["Kd"]->setFloat(Kd.x, Kd.y, Kd.z);
    material["e"]->setFloat(exp);
    material["ref"]->setFloat(ref.x, ref.y, ref.z);

	return material;
}

GeometryGroup Scene::loadOBJ(const char* filename, Material material) {
	GeometryGroup geometry = context->createGeometryGroup();
	ObjLoader loader(filename, context, geometry, material);
	//TODO intersectprogram
	loader.load();
	return geometry;
}

GeometryInstance Scene::createParallelogram(float3 v1, float3 v2, float3 anchor, Material material) {
	Geometry geom = context->createGeometry();
	geom->setPrimitiveCount( 1u );
	geom->setBoundingBoxProgram(context->createProgramFromPTXFile(PARALLELOGRAM_PTX, "bounds"));
	geom->setIntersectionProgram(context->createProgramFromPTXFile(PARALLELOGRAM_PTX, "intersect"));

	float3 normal = normalize(cross(v1, v2));
	float d = dot(normal, anchor);
	v1 *= 1.0f/dot(v1, v1);
	v2 *= 1.0f/dot(v2, v2);
	float4 plane = make_float4(normal, d);
	geom["plane"]->setFloat(plane.x, plane.y, plane.z, plane.w);
	geom["v1"]->setFloat(v1.x, v1.y, v1.z);
	geom["v2"]->setFloat(v2.x, v2.y, v2.z);
	geom["anchor"]->setFloat(anchor.x, anchor.y, anchor.z);

	GeometryInstance instance = context->createGeometryInstance();
	instance->setGeometry(geom);
	instance->setMaterialCount(1);
	instance->setMaterial(0, material);
	return instance;
}

void Scene::addLight(float3 pos, float3 color) {
	BasicLight light = {pos, color, 1};
	lights.push_back(light);
}

void Scene::render() {
	Buffer buffer = context->createBuffer(RT_BUFFER_INPUT);
	buffer->setFormat(RT_FORMAT_USER);
	buffer->setElementSize(sizeof(BasicLight));
	buffer->setSize(lights.size());
	memcpy(buffer->map(), lights.data(), sizeof(BasicLight) * lights.size());
	buffer->unmap();
	context["lights"]->set(buffer);

	context->validate();
    context->compile();
    context->launch(0, width, height);
	context->checkError(sutilDisplayBufferInGlutWindow("Scene", context["output_buffer"]->getBuffer()->get()));
}