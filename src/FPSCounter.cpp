#include "FPSCounter.h"
#include <GLFW/glfw3.h>
#include <sstream>
#include <stdio.h>

FPSCounter::FPSCounter(double delay, double alpha) {
	this->delay = delay;
	this->alpha = alpha;
	reset();
}

void FPSCounter::setReportDelay(double delay) {
	this->delay = delay;
}

void FPSCounter::setAlpha(double alpha) {
	this->alpha = alpha;
}

void FPSCounter::reset() {
	curr = glfwGetTime();
	time = 0.0;
	report = delay;
}

double FPSCounter::nextFrame() {
	prev = curr;
	curr = glfwGetTime();
	double dt = curr - prev;
	time += alpha * (dt - time);	//exponential moving average
	if (delay >= 0 && (report -= dt) <= 0) {
		printf("Avg: %.5fms (%.1ffps)\n", getAverageFrameTime(), getAverageFPS());
		report = delay;
	}	
	return dt;
}

double FPSCounter::getAverageFrameTime() {
	return time;
}

double FPSCounter::getAverageFPS() {
	return 1.0 / time;
}