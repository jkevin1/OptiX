#pragma once

/*	
 *	Contains constant values and structs that can be used by both
 *	the main program and CUDA files.
 */

//Number of different ray types, followed by the ID's of each
#define NUM_RAYS 2
#define RADIANCE_RAY 0
#define SHADOW_RAY 1

//Maximum number of recursive rays that can be spawned
#define MAX_DEPTH 5

//Minimum t for ray intersection
#define DEFAULT_MIN 1.0e-3f

//Simple struct for a light, from SDK
typedef struct struct_BasicLight {
  float3 pos;
  float3 color;
  int    casts_shadow; 
  int    padding;	//32 byte total size
} BasicLight;

//Structs to contain data about ray intersection
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