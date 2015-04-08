#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#define DEFAULT_MIN 1e-5
#define DEFAULT_MAX 1e+5

using namespace optix;

struct RayData {
	float3 result;
	int E, id;	//used?
	int depth;
	unsigned seed;
	//curand_uniform
};

// Initialize random seed
static __device__ __inline__ unsigned int createSeed(unsigned val0, unsigned val1) {
	unsigned v0 = val0;
	unsigned v1 = val1;
	unsigned s0 = 0;

	for (unsigned n = 0; n < 16; n++) {
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}

	return v0;
}

// Generate random unsigned int in [0, 2^24)
static __device__ __inline__ unsigned rand(unsigned &prev) {
	const unsigned int LCG_A = 1664525u;
	const unsigned int LCG_C = 1013904223u;
	prev = (LCG_A * prev + LCG_C);
	return prev & 0x00FFFFFF;
}

// Generate random float in [0, 1)
static __device__ __inline__ float randFloat(unsigned &prev) {
	return ((float)rand(prev) / (float)0x01000000);
}