#include "RayData.h"
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>

rtBuffer<uchar4, 2> output;
rtDeclareVariable(uint2, index, rtLaunchIndex, );
rtDeclareVariable(uint2, dim, rtLaunchDim, );
rtDeclareVariable(rtObject, objects, , );
rtDeclareVariable(int, samples, , );
rtDeclareVariable(float3, camOrigin, , );
rtDeclareVariable(float3, camDirection, , );

static __device__ __inline__ float3 radiance(float3 o, float3 d, int depth, unsigned& seed) {
	//return make_float3(randFloat(seed), randFloat(seed), randFloat(seed));
	RayData payload;
	payload.E = 1;
	payload.depth = depth;
	payload.seed = seed;
	optix::Ray ray = optix::make_Ray(o, d, 0, DEFAULT_MIN, DEFAULT_MAX);
	rtTrace(objects, ray, payload);
	return payload.result;
}

static __device__ __inline__ unsigned char correct(float color) {
	return static_cast<unsigned char>(powf(clamp(color, 0.0f, 1.0f), 1 / 2.2) * 255 + .5);
}

static __device__ __inline__ uchar4 convertColor(float3 clr) {
	return make_uchar4(correct(clr.z), correct(clr.y), correct(clr.x), 255u);
}

RT_PROGRAM void trace() {
	float3 cx = make_float3(dim.x*0.5135f / dim.y, 0.0f, 0.0f);
	float3 cy = normalize(cross(cx, camDirection)) * 0.5135f;
	unsigned seed = createSeed(dim.x*index.y + index.x, dim.y*index.x + index.y);
	float3 result = make_float3(0.0f, 0.0f, 0.0f);
	for (int sy = 0; sy < 2; sy++) {		// 2x2 subpixel rows
		for (int sx = 0; sx < 2; sx++) {				// 2x2 subpixel cols
			float3 r = make_float3(0.0f, 0.0f, 0.0f);
			for (int s = 0; s < samples; s++) {
				float r1 = 2 * randFloat(seed), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
				float r2 = 2 * randFloat(seed), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
				float3 d =	cx*(((sx + 0.5f + dx) / 2 + index.x) / dim.x - 0.5f) +
							cy*(((sy + 0.5f + dy) / 2 + index.y) / dim.y - 0.5f) + camDirection;
				r += radiance(camOrigin + d * 140, normalize(d), 0, seed);
			} // Camera rays are pushed ^^^^^ forward to start in interior
			result += clamp(r / samples, 0.0f, 1.0f);
		}
	}
	output[index] = convertColor(result * 0.25f);
}

RT_PROGRAM void exception() {
	const unsigned int code = rtGetExceptionCode();
	rtPrintf("Caught exception 0x%X at launch index (%d,%d)\n", code, index.x, index.y);
	output[index] = convertColor(make_float3(1.0f, 0.0f, 1.0f));
}

rtDeclareVariable(RayData, data, rtPayload, );

RT_PROGRAM void miss() {
	data.result = make_float3(1.0f, 0.0f, 0.0f);
}