#include <optix_device.h>
#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

rtDeclareVariable(float3, Po, , );
rtDeclareVariable(float3, N, , );
rtDeclareVariable(float3, R, , );
rtDeclareVariable(int, objID, , );

rtDeclareVariable(Ray, ray, rtCurrentRay, );

rtDeclareVariable(float3, n, attribute normal, );
rtDeclareVariable(int, id, attribute ID, );

RT_PROGRAM void intersect(int primIdx) {
	float3 Lo = ray.origin;
	float3 L = ray.direction;
	float t = dot(Po - Lo, N) / dot(L, N);
	if (rtPotentialIntersection(t)) {
		n = N;
		id = objID;
		rtReportIntersection(0);
	}
}

RT_PROGRAM void bounds(int, float result[6]) {
	optix::Aabb* aabb = (optix::Aabb*)result;
	if (length(N) > 0 && length(R) > 0) {
		aabb->m_max = Po + R;
		aabb->m_min = Po - R;
		return;
	}
	aabb->invalidate();
}