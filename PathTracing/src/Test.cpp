#include "TestScene.h"
#include <chrono>

#pragma warning( disable : 4996 )

#define QUAL_ITERATIONS 100
#define TIME_ITERATIONS 24
#define MAX_SAMPLES 128

using namespace std::chrono;

//Test
int main(int argc, char* argv[]) {
	try {
		TestScene scene(512, 512);
		//First render to handle first-time tasks like copying data
		scene.setNumSamples(1);
		scene.setMaxDepth(1);
		scene.render();


		printf("Image quality test\n");
		char filename[128];
		scene.setNumSamples(32);
		scene.setMaxDepth(4);
		for (int i = 1; i <= QUAL_ITERATIONS; i++) {
			sprintf(filename, "tests/32x%d.ppm", i);
			scene.setFrame(i);
			scene.render();
			scene.saveScreenshot(filename, i);
			printf("\r%f%%", 100.0f * float(i) / QUAL_ITERATIONS);
		}

		printf("\nImage time test\n");
		FILE* out = fopen("tests/times.txt", "w");
		for (int s = 1; s < MAX_SAMPLES; s++) {
			scene.setNumSamples(s);
			auto time = high_resolution_clock::now();
			for (int i = 0; i < TIME_ITERATIONS; i++) {
				scene.setFrame(i);
				scene.render();
			}
			duration<double> value = duration_cast<duration<double>>(high_resolution_clock::now() - time);
			fprintf(out, "%4d samples: %f sec\n", s, value.count() / TIME_ITERATIONS);
			printf("\r%f%%", 100.0f * float(s+1) / MAX_SAMPLES);
		}
		printf("\n");
		fclose(out);


	} catch (Exception& e) {
		fprintf(stderr, "%s\n", e.getErrorString().c_str());
		return 1;
	}
}