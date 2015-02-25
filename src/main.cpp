#include "Scene.h"
#include <stdio.h>

#define TEAPOT_SCENE

#define WIDTH 1024u
#define HEIGHT 768u

int main(int argc, char* argv[]) {
	try {
		Scene::init(&argc, argv);
		Scene scene(WIDTH, HEIGHT, make_float3(-150.0f, 75.0f, 125.0f), make_float3(0.0f, 0.0f, 0.0f), make_float3(0.0f, 1.0f, 0.0f), 60.0f); 
		//add stuff

		float3 Ks = make_float3(0.6f, 0.6f, 0.6f);
		float3 Ka = make_float3(0.6f, 0.0f, 0.0f);
		float3 Kd = make_float3(0.5f, 0.0f, 0.0f);
		float3 ref= make_float3(0.0f, 0.0f, 0.0f);
		float exp = 200.0f;
		Material red = scene.createPhongMaterial(Ks, Ka, Kd, exp, ref);

		Ka = make_float3(0.6f, 0.6f, 0.6f);
		Kd = make_float3(0.8f, 0.8f, 0.8f);
		ref= make_float3(0.3f, 0.3f, 0.3f);
		Material shiny = scene.createPhongMaterial(Ks, Ka, Kd, exp, ref);

#ifdef TEAPOT_SCENE
		GeometryGroup teapot = scene.loadOBJ("teapot.obj", red);
		GeometryInstance floor = scene.createParallelogram(make_float3(300, 0, 0), make_float3(0, 0, -300), make_float3(-150, 0, 150), shiny);
		GeometryGroup group = scene.context->createGeometryGroup();
		int children = teapot->getChildCount() + 1;
		group->setChildCount(children);
		for (int i = 0; i < children - 1; i++)
			group->setChild(i, teapot->getChild(i));
		group->setChild(children - 1, floor);
		group->setAcceleration(scene.context->createAcceleration("Bvh", "BvhSingle"));
		scene.context["objects"]->set(group);
		scene.addLight(make_float3(-500.0f, 100.0f, -100.0f), make_float3(0.8f, 0.8f, 0.8f));
#else //sponza
		float3 eye = make_float3(0.0f, 5.0f, 1.0f);
		float3 lookdir = normalize(make_float3(-5.0f, 5.0f,-1.0f) - eye);
		float3 camera_u = cross(lookdir, make_float3(0.0f, 1.0f, 0.0f));
		float3 camera_v = cross(camera_u, lookdir);
		float ulen = tanf(75.0f / 2.0f * M_PIf / 180.0f);
		camera_u = normalize(camera_u);
		camera_u *= ulen;
		float aspect_ratio = float(WIDTH) / float(HEIGHT);
		float vlen = ulen / aspect_ratio;
		camera_v = normalize(camera_v);
		camera_v *= vlen;
		scene.context["eye"]->setFloat(eye.x, eye.y, eye.z);
		scene.context["U"]->setFloat(camera_u.x, camera_u.y, camera_u.z);
		scene.context["V"]->setFloat(camera_v.x, camera_v.y, camera_v.z);
		scene.context["W"]->setFloat(lookdir.x, lookdir.y, lookdir.z);
		scene.context["objects"]->set(scene.loadOBJ("sponza.obj", red));
		scene.addLight(make_float3(5.0f, 15.0f, -1.0f), make_float3(0.8f, 0.8f, 0.8f));
#endif
		scene.render();
	} catch(Exception& e) {
		fprintf(stderr, "%s\n", e.getErrorString().c_str());
		system("pause");
		return 1;
	}
	return 0;
}