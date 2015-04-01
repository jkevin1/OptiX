#include <optix_device.h>
#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>

#define T_MIN 1e-4
#define T_MAX 1e+4
#define M_PI 3.141592653589793

using namespace optix;

struct __align__(16) RayData {
	float3 result;
	unsigned depth;
	unsigned* seed;
};

rtBuffer<float4, 2> image;
rtDeclareVariable(uint2, dim, rtLaunchDim, );
rtDeclareVariable(uint2, pixel, rtLaunchIndex, );
rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3, U, , );
rtDeclareVariable(float3, V, , );
rtDeclareVariable(float3, W, , );
rtDeclareVariable(rtObject, objects, , );
rtDeclareVariable(int, NUM_SAMPLES, , );
rtDeclareVariable(int, MAX_DEPTH, , );

rtDeclareVariable(float, t, rtIntersectionDistance, );
rtDeclareVariable(RayData, current_prd, rtPayload, );
rtDeclareVariable(Ray, current_ray, rtCurrentRay, );
rtDeclareVariable(float3, n, attribute normal, );
rtDeclareVariable(float3, e, attribute emission, );
rtDeclareVariable(float3, f, attribute color, );


// Initialize random seed
static __device__ __inline__ unsigned createSeed(unsigned v0, unsigned v1) {
	for (unsigned n = 0, s0 = 0; n < 16; n++) {
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}
	return v0;
}

// LCG normalize from 0 to 1
static __device__ __inline__ float rand(unsigned& seed) {
	const unsigned int LCG_A = 1664525u;
	const unsigned int LCG_C = 1013904223u;
	seed = (LCG_A * seed + LCG_C);
	return float(seed & 0x00FFFFFF) / float(0x01000000);
}

RT_PROGRAM void pathTrace() {
	uint2 dim = image.size();
	float2 invDim2 = 2.0 / make_float2(dim);
	unsigned seed = createSeed(pixel.y*pixel.y*pixel.x, pixel.x*pixel.x*pixel.y);	// TODO: optimize seed/lcg
	float3 acc = make_float3(0.0f, 0.0f, 0.0f);
	for (int sample = 0; sample < NUM_SAMPLES; sample++) {
		float2 point = (make_float2(pixel) + make_float2(rand(seed), rand(seed))) * invDim2 - 1.0f;	// Random pixel in scene
		float3 dir = normalize(point.x * U + point.y * V + W);
		Ray ray = make_Ray(eye, dir, 0, T_MIN, T_MAX);
		RayData new_prd;
		new_prd.depth = 0;
		new_prd.seed = &seed;
		rtTrace(objects, ray, new_prd);
		acc += new_prd.result;
	}
	acc *= 1.0f / float(NUM_SAMPLES);
	image[pixel] = make_float4(clamp(acc, 0.0f, 1.0f), 1.0f);
}

RT_PROGRAM void exception() {
	const unsigned int code = rtGetExceptionCode();
	rtPrintf("Caught exception 0x%X at launch index (%d, %d)\n", code, pixel.x, pixel.y);
	image[pixel] = make_float4(1.0f, 0.0f, 1.0f, 1.0f);
}


RT_PROGRAM void miss() {
	current_prd.result = make_float3(0.0f, 0.0f, 0.0f);
}

RT_PROGRAM void diffuse() {
	if (current_prd.depth < MAX_DEPTH && length(e) < 1) {
		
		// Pick a random direction from here and keep going.
		float3 normal = n;
		float3 tangent = normalize(cross(n, make_float3(0, 1, n.z < 0 ? 1 : -1)));
		float3 binormal = normalize(cross(n, tangent));

		float theta = 2.0f * M_PI * rand(*current_prd.seed);
		float phi = 0.5f * M_PI *rand(*current_prd.seed);

		float3 pos = current_ray.origin + t * current_ray.direction;
		float3 dir = cosf(phi) * (cosf(theta) * tangent + sinf(theta) * binormal) + sinf(phi) * normal;

		Ray ray = make_Ray(pos, normalize(dir), 0, T_MIN, T_MAX);
		RayData new_prd;
		new_prd.depth = current_prd.depth + 1;
		new_prd.seed = current_prd.seed;

		// Compute the BRDF for this ray (assuming Lambertian reflection)
		float cos_theta = dot(dir, normal);
		float3 BRDF = 2 * f * cos_theta;
		rtTrace(objects, ray, new_prd);

		// Apply the Rendering Equation here.
		current_prd.result = e + (BRDF * new_prd.result);
		return;
	}
	current_prd.result = e;
}