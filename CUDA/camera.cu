#include <optix_world.h>
#include <optix_device.h>
#include "Common.h"

using namespace optix;

rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3, U, , );
rtDeclareVariable(float3, V, , );
rtDeclareVariable(float3, W, , );
rtDeclareVariable(rtObject, objects, , );

rtBuffer<uchar4, 2> output_buffer;
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );
rtDeclareVariable(uint2, launch_dim, rtLaunchDim, );

__device__ float correct(float color) {
	return clamp(color, 0.0f, 1.0f);
}

__device__ uchar4 convertColor(float3 clr) {
	return make_uchar4(static_cast<unsigned char>(correct(clr.z) * 255.99f),  /* B */
					   static_cast<unsigned char>(correct(clr.y) * 255.99f),  /* G */
					   static_cast<unsigned char>(correct(clr.x) * 255.99f),  /* R */
					   255u);
}

#define NUM_SAMPLES 4

RT_PROGRAM void raygen() {
	//Rotated-Grid AA
	float2 offset[] = {
	//	make_float2(0.0f, 0.0f),
		make_float2(1/8.0f, 3/8.0f),
		make_float2(3/8.0f, 1/8.0f),
		make_float2(5/8.0f, 7/8.0f),
		make_float2(7/8.0f, 5/8.0f)
	};

	float3 color = make_float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < NUM_SAMPLES; i++) {
		float2 d = (make_float2(launch_index) + offset[i]) / make_float2(launch_dim) * 2.0f - 1.0f;
		float3 dir = normalize(d.x*U + d.y*V + W);
  
		optix::Ray ray = optix::make_Ray(eye, dir, RADIANCE_RAY, DEFAULT_MIN, 1e8);

		PerRayData_radiance data;
		data.importance = 1.0f;
		data.depth = 0;

		rtTrace(objects, ray, data);

		color += data.result;
	}
	
	output_buffer[launch_index] = convertColor(color / NUM_SAMPLES);
}

rtDeclareVariable(float3, exception_color, , );

RT_PROGRAM void exception() {
	const unsigned int code = rtGetExceptionCode();
	rtPrintf("Caught exception 0x%X at launch index (%d,%d)\n", code, launch_index.x, launch_index.y);
	output_buffer[launch_index] = convertColor(exception_color);
}

//rtDeclareVariable(float3, background, , );
rtDeclareVariable(PerRayData_radiance, data, rtPayload, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtTextureSampler<float4, 2> envmap;

RT_PROGRAM void miss() {
//	data.result = background;
	float theta = atan2f(ray.direction.x, ray.direction.z);
	float phi = M_PIf * 0.5f - acosf(ray.direction.y);
	float u = (theta + M_PIf) * (0.5f * M_1_PIf);
	float v = 0.5f * (1.0f + sin(phi));
	data.result = make_float3(tex2D(envmap, u, v));
}