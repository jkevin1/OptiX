#include <optix_device.h>
#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>

#define M_PI 3.141592653589793

using namespace optix;

// Initialize random seed using tiny encription algorithm with 16 iterations
static __device__ __inline__ unsigned createSeed(unsigned v0, unsigned v1) {
	for (unsigned n = 0, s0 = 0; n < 16; n++) {
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}
	return v0;
}

// LCG normalized from 0 to 1
static __device__ __inline__ float rand(unsigned& seed) {
	const unsigned int LCG_A = 1664525u;
	const unsigned int LCG_C = 1013904223u;
	seed = (LCG_A * seed + LCG_C);
	return float(seed & 0x00FFFFFF) / float(0x01000000);
}

// Generate 2D random vector distributed uniformly on a disk with radius 1
static __device__ __inline__ float2 uniformSampleDisk(unsigned& seed) {
	const float u1 = rand(seed);
	const float r = sqrtf(u1);
	const float theta = 2.0f * M_PI * rand(seed);

	const float x = r * cosf(theta);
	const float y = r * sinf(theta);
}

// Generate a random sample based on the input normal with a cosine distribution
static __device__ __inline__ float3 cosineSample(const float3& normal, unsigned& seed) {
	const float u1 = rand(seed);
	const float r = sqrtf(u1);
	const float theta = 2.0f * M_PI * rand(seed);

	const float x = r * cosf(theta);
	const float y = r * sinf(theta);
	const float z = sqrtf(1.0f - u1);	// u1 is in [0, 1)

	float3 tangent = normalize(cross(normal, make_float3(0, 1, normal.z < 0 ? 1 : -1)));
	float3 binormal = normalize(cross(normal, tangent));
	return (x * tangent) + (z * normal) + (y * binormal);
}