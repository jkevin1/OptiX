#include <optix_world.h>
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

rtDeclareVariable(float4, sphere, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(Ray, ray, rtCurrentRay, );

RT_PROGRAM void intersect(int primIdx) {
	float3 O = ray.origin;
	float3 D = ray.direction;
	float3 C = make_float3(sphere);

	float a = dot(D, D);
	float b = 2.0f * dot(D, O - C);
	float c = dot(O - C, O - C) - (sphere.w * sphere.w);

	float d = (b*b) - (4.0f*a*c);
	if (d >= 0) {
		float result = ray.tmax;
		d = sqrtf(d);
		a *= 2;
		float t = (-b + d) / a;
		if (t > 1.0e-4f) result = t;
		t = (-b - d) / a;
		if (t > 1.0e-4f) result = fminf(result, t);
		if (rtPotentialIntersection(result)) {
			float3 pos = O + result*D;
			shading_normal = geometric_normal = pos - C;
			rtReportIntersection(0);
		}
	}
}

RT_PROGRAM void bounds(int, float result[6]) {
	optix::Aabb* aabb = (optix::Aabb*)result;
	if (sphere.w > 0) {
		float3 C = make_float3(sphere);
		float3 R = make_float3(sphere.w);
		aabb->m_max = C + R;
		aabb->m_min = C - R;
	} else {
		aabb->invalidate();
	}
}