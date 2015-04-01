#include "CornellBox.h"
#include "OptixViewer.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
	try {
		CornellBox scene(512, 512);
		OptixViewer viewer(&scene, "Cornell Box", true);
		viewer.run();
	} catch (Exception& e) {
		fprintf(stderr, "%s\n", e.getErrorString().c_str());
		return 1;
	}
}