#include <optix_device.h>
#include <optix_world.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

rtDeclareVariable(float, radius, , );
rtDeclareVariable(float3, position, , );

rtDeclareVariable(Ray, ray, rtCurrentRay, );

rtDeclareVariable(float3, n, attribute normal, );

RT_PROGRAM void intersect(int primIdx) {
	float3 O = ray.origin;
	float3 D = ray.direction;
	float3 C = position;

	float a = dot(D, D);
	float b = 2.0f * dot(D, O - C);
	float c = dot(O - C, O - C) - (radius * radius);

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
			n = normalize(pos - C);
			// TODO: pass other attributes to shader
			rtReportIntersection(0);
		}
	}
}

RT_PROGRAM void bounds(int, float result[6]) {
	optix::Aabb* aabb = (optix::Aabb*)result;
	if (radius > 0) {
		float3 R = make_float3(radius);
		aabb->m_max = position + R;
		aabb->m_min = position - R;
	} else {
		aabb->invalidate();
	}
}