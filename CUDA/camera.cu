
#include <optix_world.h>
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

RT_PROGRAM void raygen() {
	float2 d = make_float2(launch_index) / make_float2(launch_dim) * 2.0f - 1.0f;
	float3 dir = normalize(d.x*U + d.y*V + W);
  
	optix::Ray ray = optix::make_Ray(eye, dir, RADIANCE_RAY, DEFAULT_MIN, RT_DEFAULT_MAX);

	PerRayData_radiance data;
	data.importance = 1.0f;
	data.depth = 0;

	rtTrace(objects, ray, data);

	output_buffer[launch_index] = optix::make_uchar4(static_cast<unsigned char>(__saturatef(data.result.z)*255.99f),  /* B */
													 static_cast<unsigned char>(__saturatef(data.result.y)*255.99f),  /* G */
													 static_cast<unsigned char>(__saturatef(data.result.x)*255.99f),  /* R */
													 255u);
}

rtDeclareVariable(float3, exception_color, , );

RT_PROGRAM void exception() {
	const unsigned int code = rtGetExceptionCode();
	rtPrintf("Caught exception 0x%X at launch index (%d,%d)\n", code, launch_index.x, launch_index.y);
	output_buffer[launch_index] = optix::make_uchar4(static_cast<unsigned char>(__saturatef(exception_color.z)*255.99f),  /* B */
													 static_cast<unsigned char>(__saturatef(exception_color.y)*255.99f),  /* G */
													 static_cast<unsigned char>(__saturatef(exception_color.x)*255.99f),  /* R */
													 255u);
}

rtDeclareVariable(float3, background, , );
rtDeclareVariable(PerRayData_radiance, data, rtPayload, );

RT_PROGRAM void miss() {
	data.result = background;
}