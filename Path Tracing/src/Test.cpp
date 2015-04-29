#include "TestScene.h"
#include "OptixViewer.h"
#include <chrono>

#pragma warning( disable : 4996 )

#define QUAL_ITERATIONS 100
#define TIME_ITERATIONS 256

#define WIDTH 512
#define HEIGHT 512
#define SAMPLES 6
#define DEPTH 4

using namespace std::chrono;

//Test
int main(int argc, char* argv[]) {
	try {
		TestScene scene(WIDTH, HEIGHT, SAMPLES, DEPTH);
		OptixViewer viewer(&scene, "Path Tracer");
		viewer.run();
	} catch (Exception& e) {
		fprintf(stderr, "Error:\n%s\n", e.what());
		return 1;
	}
}