#pragma once

#include <string>

/*
 *	This class represents a FPS counter which uses glfwGetTime and an exponential moving average function to
 *	calculate the average frame time.  Alpha represents the weight of the most recent frame, a higher value
 *	will cause older frames to have less of an effect.  Setting alpha = 2 / (N + 1) will make the last N frames
 *	account for approximately 86% of the average frame times.  This class can automatically report the FPS.
 */

class FPSCounter {
	double report, delay;
	double alpha, time;
	double prev, curr;
public:
	//Initializes the FPSCounter, defaults to not reporting and using the last 60 frames as 86% of the average
	FPSCounter(double delay = -1.0, double alpha = 2.0 / 61.0);

	//Sets the report delay in seconds, negative value disables reporting
	void setReportDelay(double delay);

	//Sets the alpha value, it is recommended to call reset() after
	void setAlpha(double alpha);

	//Resets variables to initial state
	void reset();

	//Called each frame, updates the average frame time, reports if necessary, returns time since previous call
	double nextFrame();

	//Returns the average frame time
	double getAverageFrameTime();

	//Returns the average FPS
	double getAverageFPS();

	//Returns fps as a string
	std::string getAverageFPSString();
};