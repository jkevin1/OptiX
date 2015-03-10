#include <optix_world.h>
#include "algebra.h"

using namespace optix;

rtDeclareVariable(float2, radii, , );	//major, minor

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(Ray, ray, rtCurrentRay, );

RT_PROGRAM void intersect(int primIdx) {
	float3 O = ray.origin;
	float3 D = ray.direction;
	float ab = dot(O, D);
	float aa = dot(O, O);

	float R2 = radii.x*radii.x;
	float r2 = radii.y*radii.y;
	float K = aa - r2 - R2;
	float constants[5], roots[4];
	constants[4] = 1.0f;
	constants[3] = 4*ab;
	constants[2] = 2*(2*ab*ab + K + 2*R2*D.z*D.z);
	constants[1] = 4*(K*ab + 2*R2*O.z*D.z);
	constants[0] = K*K + 4*R2*(O.z*O.z - r2);
	int nroots = solveQuartic(constants, roots);

	int intersection = 0;
	float min = ray.tmax;
	while(nroots--) {
		float t = roots[nroots];
		//float x = O.x + t*D.x;
		//float y = O.y + t*D.y;
		//float l = radii.x*(M_PI/2 - atan2(y,x));
		//if (/*l <= vlength && */l >= 0) {
			if (t < min) {
				if (rtPotentialIntersection(t)) {
					min = t;
					intersection = 1;
				}
			}
		//}
	}
	if (intersection) {
		float3 pos = O + min*D;
		float3 rel = pos; rel.y = 0;
		rel = radii.x * normalize(rel);
		shading_normal = geometric_normal = pos - rel;
        rtReportIntersection(0);
	}
}

RT_PROGRAM void bounds(int, float result[6]) {
	optix::Aabb* aabb = (optix::Aabb*)result;
	if (radii.x > 0 || radii.y > 0) {
		aabb->m_max = make_float3(radii.x + radii.y, radii.x + radii.y, radii.x + radii.y);
		aabb->m_max = aabb->m_max * aabb->m_max;
		aabb->m_min = -aabb->m_max;
	} else {
		aabb->invalidate();
	}
}