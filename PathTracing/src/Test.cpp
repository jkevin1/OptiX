#include "TestScene.h"
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

		printf("Image quality test\n");
		char filename[128];
		for (int i = 1; i <= QUAL_ITERATIONS; i++) {
			sprintf(filename, "tests/frame%d.ppm", i);
			scene.render();
			scene.saveScreenshot(filename);
			printf("\r%f%%", 100.0f * float(i) / QUAL_ITERATIONS);
		}
		printf("\n");

		printf("Image time test\n");
		FILE* out = fopen("tests/times.txt", "w");
		auto time = high_resolution_clock::now();
		for (int i = 0; i < TIME_ITERATIONS; i++)
			scene.render();
		duration<double> value = duration_cast<duration<double>>(high_resolution_clock::now() - time);
		fprintf(out, "[%dx%d|%d samples|%d depth]: %f sec\n", WIDTH, HEIGHT, SAMPLES, DEPTH, value.count() / TIME_ITERATIONS);
		printf("\n");
		fclose(out);	


	} catch (Exception& e) {
		fprintf(stderr, "%s\n", e.what());
		return 1;
	}
}