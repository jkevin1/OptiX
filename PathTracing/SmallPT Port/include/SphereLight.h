#include <optix.h>
#include <optixu/optixu_math_namespace.h>

struct SphereLight {
	optix::float3 p;
	optix::float3 e;
	float r;
	int id;
};