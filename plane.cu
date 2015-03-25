#include <optix_world.h>
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

rtDeclareVariable(float3, Po, , );
rtDeclareVariable(float3, N, , );
rtDeclareVariable(float, R, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(Ray, ray, rtCurrentRay, );

RT_PROGRAM void intersect(int primIdx) {
	float3 Lo = ray.origin;
	float3 L = ray.direction;
	float t =  dot(Po-Lo, N) / dot(L, N);
	if (rtPotentialIntersection(t)) {
		geometric_normal = shading_normal = N;
		rtReportIntersection(0);
	}
}

RT_PROGRAM void bounds(int, float result[6]) {
	optix::Aabb* aabb = (optix::Aabb*)result;
	if (length(N) > 0 && R > 0) {
		float3 rad = make_float3(R, 0.0f, R);
		aabb->m_max = Po + rad;
		aabb->m_min = Po - rad;
	} else {
		aabb->invalidate();
	}
}