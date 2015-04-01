#include "TestScene.h"
#include <chrono>

#pragma warning( disable : 4996 )

using namespace std::chrono;

//Test
int main(int argc, char* argv[]) {
	try {
		TestScene scene(512, 512);
		//First render to handle first-time tasks like copying data
		scene.setNumSamples(1);
		scene.setMaxDepth(1);
		scene.render();

		FILE* timing = fopen("tests/timing.txt", "w");
		fprintf(timing, "samples\tdepth\ttime\n");

		char filename[64];
		int samples = 1;
		for (int i = 0; i < 14; i++) {
			scene.setNumSamples(samples);
			printf("\rRunning tests with %d samples\n", samples);
			for (int depth = 2; depth < 8; depth++) {
				printf("\r\t%d max depth", depth);
				scene.setMaxDepth(depth);
				auto time = high_resolution_clock::now();
				scene.render();
				duration<double> value = duration_cast<duration<double>>(high_resolution_clock::now() - time);
				fprintf(timing, "%7d\t%5d\t%f seconds\n", samples, depth, value.count());
				sprintf(filename, "tests/%ds%dd.ppm", samples, depth);
				scene.saveScreenshot(filename);
			}
			samples *= 2;
		}

		fclose(timing);
	} catch (Exception& e) {
		fprintf(stderr, "%s\n", e.getErrorString().c_str());
		return 1;
	}
}