#pragma once

#define RADIANCE_RAY 0
#define SHADOW_RAY 1
#define MAX_DEPTH 5
#define DEFAULT_MIN 1.0e-3f

typedef struct struct_BasicLight
{
#if defined(__cplusplus)
  typedef optix::float3 float3;
#endif
  float3 pos;
  float3 color;
  int    casts_shadow; 
  int    padding;      // make this structure 32 bytes -- powers of two are your friend!
} BasicLight;

#ifdef __CUDACC__
struct PerRayData_radiance {
	float3 result;
	float  importance;
	int    depth;
};

struct PerRayData_shadow {
  float3 attenuation;
};
#endif