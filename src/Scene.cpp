#include "Scene.h"

Scene::Scene(unsigned width, unsigned height, unsigned vbo) : context(createContext(NUM_RAYS)), width(width), height(height) {
	//Create the output buffer from vbo
	Buffer buffer = context->createBufferFromGLBO(RT_BUFFER_OUTPUT, vbo);
    buffer->setFormat(RT_FORMAT_UNSIGNED_BYTE4);
    buffer->setSize(width, height);
	context["output_buffer"]->set(buffer);
	
	//Load ray generation, exception, and miss programs
	std::string camera_ptx = "camera.cu.ptx";
	context->setRayGenerationProgram(0, context->createProgramFromPTXFile(camera_ptx, "raygen"));
	context->setExceptionProgram(0, context->createProgramFromPTXFile(camera_ptx, "exception"));
	context->setMissProgram(0, context->createProgramFromPTXFile(camera_ptx, "miss"));
	context["exception_color"]->setFloat(1.0f, 0.0f, 1.0f);
	context["envmap"]->setTextureSampler(loadTexture(context, "env.png"));
}

Scene::~Scene() {
	context->destroy();
}

void Scene::setCamera(float3 eye, float3 at, float3 up, float hfov) {
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

void Scene::addLight(float3 pos, float3 color) {
	BasicLight light = {pos, color, 1};
	lights.push_back(light);
}

GeometryInstance& Scene::addObject(Geometry type, Material material) {
	GeometryInstance object = context->createGeometryInstance();
	object->setGeometry(type);
	object->setMaterialCount(1);
	object->setMaterial(0, material);
	objects.push_back(object);
	return objects[objects.size()-1];
}

GeometryInstance& Scene::addObject(GeometryInstance object) {
	objects.push_back(object);
	return objects[objects.size()-1];
}

GeometryInstance& Scene::addObject(GeometryGroup object) {
	unsigned num = object->getChildCount();
	for (unsigned i = 0; i < num; i++)
		objects.push_back(object->getChild(i));
	return objects[objects.size()-num];
}

void Scene::compile() {
	//Prepare lights array
	Buffer buffer = context->createBuffer(RT_BUFFER_INPUT);
	buffer->setFormat(RT_FORMAT_USER);
	buffer->setElementSize(sizeof(BasicLight));
	buffer->setSize(lights.size());
	memcpy(buffer->map(), &lights[0], sizeof(BasicLight) * lights.size());
	buffer->unmap();
	context["lights"]->set(buffer);

	//Prepare objects array
	GeometryGroup group = context->createGeometryGroup();
	group->setChildCount(objects.size());
	for (unsigned i = 0; i < objects.size(); i++)
		group->setChild(i, objects[i]);
	group->setAcceleration(context->createAcceleration("Bvh", "BvhSingle"));
	//group->setAcceleration(context->createAcceleration("NoAccel", "NoAccel"));	//plane bounding box isnt working? what?
	context["objects"]->set(group);

	//Validate and send everything to the GPU for rendering
	context->validate();
    context->compile();
}

void Scene::render() {
    context->launch(0, width, height);
}