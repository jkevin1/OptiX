#include <optix.h>
#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>

struct __align__(16) PotentialLight {
	optix::float3 position;
	float radius;
};