#include <optix_world.h>
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
//#include "algebra.h"

using namespace optix;

rtDeclareVariable(float2, radii, , );	//major, minor

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 
rtDeclareVariable(Ray, ray, rtCurrentRay, );
/*
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
		//if (l >= 0) {
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
		float3 rel = pos; rel.z = 0;
		rel = radii.x * normalize(rel);
		shading_normal = geometric_normal = pos - rel;
        rtReportIntersection(0);
	}
}
*/
RT_PROGRAM void intersect(int primIdx) {
	float3 ro = ray.origin;
	float3 rd = ray.direction;

	float Ra2 = radii.x*radii.x;
	float ra2 = radii.y*radii.y;
	
	float m = dot(ro,ro);
	float n = dot(ro,rd);
		
	float k = (m - ra2 - Ra2)/2.0;
	float a = n;
	float b = n*n + Ra2*rd.z*rd.z + k;
	float c = k*n + Ra2*ro.z*rd.z;
	float d = k*k + Ra2*ro.z*ro.z - Ra2*ra2;
	
    //----------------------------------

	float p = -3.0*a*a     + 2.0*b;
	float q =  2.0*a*a*a   - 2.0*a*b   + 2.0*c;
	float r = -3.0*a*a*a*a + 4.0*a*a*b - 8.0*a*c + 4.0*d;
	p /= 3.0;
	r /= 3.0;
	float Q = p*p + r;
	float R = 3.0*r*p - p*p*p - q*q;
	
	float h = R*R - Q*Q*Q;
	float z = 0.0;
	if( h < 0.0 )
	{
		float sQ = sqrt(Q);
		z = 2.0*sQ*cos( acos(R/(sQ*Q)) / 3.0 );
	}
	else
	{
		float sQ = powf( sqrt(h) + abs(R), 1.0/3.0 );
		z = abs( sQ + Q/sQ );
		if (R < 0) z = -z;
	}
	
	z = p - z;
	
    //----------------------------------
	
	float d1 = z   - 3.0*p;
	float d2 = z*z - 3.0*r;

	if( abs(d1)<1.0e-4 )
	{
		if( d2<0.0 ) return;
		d2 = sqrt(d2);
	}
	else
	{
		if( d1<0.0 ) return;
		d1 = sqrt( d1/2.0 );
		d2 = q/d1;
	}

    //----------------------------------
	
	float result = 1e10;

	h = d1*d1 - z + d2;
	if( h>0.0 )
	{
		h = sqrt(h);
		float t1 = -d1 - h - a;
		float t2 = -d1 + h - a;
			 if( t1>1.0e-3f ) result=t1;
		else if( t2>1.0e-3f ) result=t2;
	}

	h = d1*d1 - z - d2;
	if( h>0.0 )
	{
		h = sqrt(h);
		float t1 = d1 - h - a;
		float t2 = d1 + h - a;
		     if( t1>1.0e-3f ) result=fminf(result,t1);
		else if( t2>1.0e-3f ) result=fminf(result,t2);
	}

	if (rtPotentialIntersection(result)) {
		float3 pos = ro + result*rd;
		float3 rel = pos; rel.z = 0;
		rel = radii.x * normalize(rel);
		shading_normal = geometric_normal = pos - rel;
        rtReportIntersection(0);
	}
}

RT_PROGRAM void bounds(int, float result[6]) {
	optix::Aabb* aabb = (optix::Aabb*)result;
	if (radii.x > 0 || radii.y > 0) {
		aabb->m_max = make_float3(radii.x + radii.y, radii.x + radii.y, radii.y);
		aabb->m_min = -aabb->m_max;
	} else {
		aabb->invalidate();
	}
}