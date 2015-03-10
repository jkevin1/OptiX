#include "FPSCounter.h"
#include "Viewer.h"
#include "OptiX.h"
#include "Scene.h"
#include <ObjLoader.h>
#include <stdio.h>
#include <math.h>

#define WIDTH 1024u
#define HEIGHT 768u

#define SPONZA_SCENE 0
#define TORUS_SCENE 1

void init(Scene& scene, int num);
void update(Scene& scene, float t, int num);

int main(int argc, char* argv[]) {
	Viewer viewer("OptiX Demo", WIDTH, HEIGHT);
	Scene scene(WIDTH, HEIGHT, viewer.getBuffer());

	int num = TORUS_SCENE;

	init(scene, num);

	FPSCounter fps;
	float t = 0;
	try {
		scene.compile();
	} catch(Exception& e) {
		fprintf(stderr, "%s\n", e.getErrorString().c_str());
		system("pause");
		return 1;
	}
	while (!viewer.isClosed()) {
		t += (float)fps.nextFrame();
		viewer.setTitle(fps.getAverageFPSString().c_str());
		update(scene, t, num);
		scene.render();
		viewer.render();
	}
}

void init(Scene& scene, int num) {
	switch (num) {
	case SPONZA_SCENE: {
		//Create material
		float3 Ks = make_float3(0.6f, 0.6f, 0.6f);
		float3 Ka = make_float3(0.6f, 0.0f, 0.0f);
		float3 Kd = make_float3(0.5f, 0.0f, 0.0f);
		float3 ref= make_float3(0.0f, 0.0f, 0.0f);
		float exp = 200.0f;
		Material red = createPhongMaterial(scene.context, Ks, Ka, Kd, exp, ref);

		//Load sponza obj
		GeometryGroup sponza = scene.context->createGeometryGroup();
		ObjLoader loader("sponza/sponza.obj", scene.context, sponza, red);
		loader.load();
		scene.addObject(sponza);

		//Create light
		scene.addLight(make_float3(5.0f, 15.0f, -1.0f), make_float3(0.8f, 0.8f, 0.8f));
		break;
	}
	case TORUS_SCENE: {
		//Create material
		float3 Ks = make_float3(0.6f, 0.6f, 0.6f);
		float3 Ka = make_float3(0.6f, 0.0f, 0.0f);
		float3 Kd = make_float3(0.5f, 0.0f, 0.0f);
		float3 ref= make_float3(0.0f, 0.0f, 0.0f);
		float exp = 200.0f;
		Material red = createPhongMaterial(scene.context, Ks, Ka, Kd, exp, ref);

		//Create torus geometry
		Geometry torusGeom = createGeometry(scene.context, "torus.cu.ptx");
		GeometryInstance& torus = scene.addObject(torusGeom, red);
		torus["radii"]->setFloat(6.0f, 2.0f);
		
		//Create light
		scene.addLight(make_float3(2.0f, 5.0f, 2.0f), make_float3(0.8f, 0.8f, 0.8f));
		break;
	}
	}
}

void update(Scene& scene, float t, int num) {
	switch (num) {
	case SPONZA_SCENE: {
		float3 at = make_float3(sinf(t/5), 5.0f, cosf(t/5));
		scene.setCamera(make_float3(0.0f, 5.0f, 1.0f), at, make_float3(0.0f, 1.0f, 0.0f), 65.0f);
	}
	case TORUS_SCENE: {
		float3 eye = 16 * make_float3(0.0f, sinf(t/5), cosf(t/5));
		float3 up = make_float3(0.0f, cosf(t/5), -sinf(t/5));
		scene.setCamera(eye, make_float3(0.0f, 0.0f, 0.0f), up, 65.0f);
	}
	}
}