#include "FPSCounter.h"
#include "Viewer.h"
#include "OptiX.h"
#include "Scene.h"
#include <stdio.h>
#include <math.h>

#define WIDTH 1024u
#define HEIGHT 768u

void init(Scene& scene);
void update(Scene& scene, float t);

int main(int argc, char* argv[]) {
	Viewer viewer("OptiX Demo", WIDTH, HEIGHT);
	Scene scene(WIDTH, HEIGHT, viewer.getVBO());

	init(scene);

	FPSCounter fps(2.0);
	float t = 0;
	try {
		scene.compile();
	} catch(Exception& e) {
		fprintf(stderr, "%s\n", e.getErrorString().c_str());
		system("pause");
		return 1;
	}
	while (viewer.isOpen()) {
		t += (float)fps.nextFrame();
		update(scene, t);
		scene.render();
		viewer.render();
	}
}

void init(Scene& scene) {
	float3 Ks = make_float3(0.6f, 0.6f, 0.6f);
	float3 Ka = make_float3(0.6f, 0.0f, 0.0f);
	float3 Kd = make_float3(0.5f, 0.0f, 0.0f);
	float3 ref= make_float3(0.0f, 0.0f, 0.0f);
	float opacity = 1.0f;
	float exp = 200.0f;
	Material red = createPhongMaterial(scene.context, Ks, Ka, Kd, exp, ref, opacity);

	Ks = make_float3(0.6f, 0.6f, 0.6f);
	Ka = make_float3(0.0f, 0.6f, 0.0f);
	Kd = make_float3(0.0f, 0.5f, 0.0f);
	ref= make_float3(0.0f, 0.0f, 0.0f);
	opacity = 1.0f;
	exp = 200.0f;
	Material green = createPhongMaterial(scene.context, Ks, Ka, Kd, exp, ref, opacity);

	Ks = make_float3(0.6f, 0.6f, 0.6f);
	Ka = make_float3(0.0f, 0.0f, 0.0f);
	Kd = make_float3(0.0f, 0.0f, 0.0f);
	ref= make_float3(0.1f, 0.1f, 0.1f);
	opacity = 0.2f;
	exp = 200.0f;
	Material blue = createPhongMaterial(scene.context, Ks, Ka, Kd, exp, ref, opacity, 1.5);
	
	Ks = make_float3(0.5f, 0.5f, 0.5f);
	Ka = make_float3(0.5f, 0.5f, 0.5f);
	Kd = make_float3(0.5f, 0.5f, 0.5f);
	ref= make_float3(0.8f, 0.8f, 0.8f);
	opacity = 1.0f;
	exp = 100.0f;
	Material mirror = createPhongMaterial(scene.context, Ks, Ka, Kd, exp, ref, opacity);

	//Create torus geometry
/*	Geometry torusGeom = createGeometry(scene.context, "torus.cu.ptx");
	GeometryInstance& torus = scene.addObject(torusGeom, blue);
	torus["radii"]->setFloat(1.0f, 0.5f);
	blue["mode"]->setInt(0);	*/

	//Create sphere scene
	Geometry sphereGeom = createGeometry(scene.context, "sphere.cu.ptx");
	GeometryInstance sphere = scene.addObject(sphereGeom, red);
	sphere["sphere"]->setFloat(1.0f, 0.0f, 1.0f, 0.5f);
	sphere = scene.addObject(sphereGeom, green);
	sphere["sphere"]->setFloat(-1.0f, 0.0f, 1.0f, 0.5f);
	sphere = scene.addObject(sphereGeom, blue);
	sphere["sphere"]->setFloat(0.0f, 0.0f, -1.2f, 0.5f);
	Geometry planeGeom = createGeometry(scene.context, "plane.cu.ptx");
	GeometryInstance plane = scene.addObject(planeGeom, mirror);
	plane["Po"]->setFloat(0.0f, -0.5f, 0.0f);
	plane["N"]->setFloat(0.0f, 1.0f, 0.0f);
	plane["R"]->setFloat(2.0f);

	//Create light
	scene.addLight(make_float3(4.0f, 1.0f, 2.0f), make_float3(0.8f, 0.8f, 0.8f));
}

void update(Scene& scene, float t) {
	float3 eye = make_float3(5.0f * cosf(t/5), 0.2, 5.0f * sinf(t/5));
	float3 up = make_float3(0.0f, 1.0f, 0.0f);
	scene.setCamera(eye, make_float3(0.0f, 0.2f, 0.0f), up, 65.0f);
}