#include "RayData.h"
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>

#define M_PI 3.141592653589793

using namespace optix;

rtDeclareVariable(float3, f, , );
rtDeclareVariable(float3, e, , );

rtDeclareVariable(Ray, r, rtCurrentRay, );
rtDeclareVariable(float, t, rtIntersectionDistance, );
rtDeclareVariable(RayData, data, rtPayload, );
rtDeclareVariable(rtObject, objects, , );
rtDeclareVariable(int, maxDepth, , );

rtDeclareVariable(float3, n, attribute normal, );

static __device__ __inline__ float3 radiance(float3 o, float3 d, int depth) {
	RayData payload;
	payload.depth = depth;
	payload.seed = data.seed;
	optix::Ray ray = optix::make_Ray(o, d, 0, DEFAULT_MIN, DEFAULT_MAX);
	rtTrace(objects, ray, payload);
	return payload.result;
}

RT_PROGRAM void diffuse() {		// Ideal diffuse reflection
	if (data.depth >= maxDepth) {
		data.result = e;
		return;
	}
	float3 pos = r.origin + r.direction*t;
	float3 nl = dot(n, r.direction) < 0 ? n : -n;
	float p = fmaxf(f); // max reflectance

	// Possibly optional line?
//	if (++depth>5) if (randFloat()<p) f = f*(1 / p); else return obj.e;
	
	float r1 = 2 * M_PI*randFloat(data.seed);
	float r2 = randFloat(data.seed), r2s = sqrt(r2);
	float3 w = nl;
	float3 u = normalize(cross((fabs(w.x) > .1 ? make_float3(0.0f, 1.0f, 0.0f) : make_float3(1.0f, 0.0f, 0.0f)), w));
	float3 v = cross(w, u);
	float3 d = normalize(u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1 - r2));
	data.result = e + (f * radiance(pos, d, data.depth + 1));
}

RT_PROGRAM void specular() {	// Ideal specular reflection
	if (data.depth > maxDepth) {
		data.result = e;
		return;
	}
	float3 pos = r.origin + r.direction*t;
	float3 nl = dot(n, r.direction) < 0 ? n : -n;
	float p = fmaxf(f); // max reflectance

	// Possibly optional line?
	//	if (++depth>5) if (randFloat()<p) f = f*(1 / p); else return obj.e;

	data.result = e + (f * radiance(pos, r.direction - (n * 2 * dot(n, r.direction)), data.depth + 1));
}

RT_PROGRAM void refractive() {	// Ideal dielectric REFRACTION
	if (data.depth > maxDepth) {
		data.result = e;
		return;
	}
	float3 pos = r.origin + r.direction*t;
	float3 nl = dot(n, r.direction) < 0 ? n : -n;
	float p = fmaxf(f); // max reflectance

	// Possibly optional line?
	//	if (++depth>5) if (randFloat()<p) f = f*(1 / p); else return obj.e;

	float3 reflDir = r.direction - (n * 2 * dot(n, r.direction));
	bool into = dot(n, nl) > 0;
	float nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = dot(r.direction, nl), cos2t;
	if ((cos2t = 1 - nnt*nnt*(1 - ddn*ddn)) < 0) {	// Total internal reflection
		data.result = e + (f * radiance(pos, reflDir, data.depth + 1));
	}
	float3 tdir = normalize(r.direction*nnt - n*((into ? 1 : -1)*(ddn*nnt + sqrt(cos2t))));
	float a = nt - nc, b = nt + nc, R0 = a*a / (b*b), c = 1 - (into ? -ddn : dot(tdir, n));
	float Re = R0 + (1 - R0)*c*c*c*c*c, Tr = 1 - Re, P = .25 + .5*Re, RP = Re / P, TP = Tr / (1 - P);
	data.result = e + (f * (data.depth > 2 ? (randFloat(data.seed) < P ?   // Russian roulette
		radiance(pos, reflDir, data.depth + 1)*RP : radiance(pos, tdir, data.depth + 1)*TP) :
		radiance(pos, reflDir, data.depth + 1)*Re + radiance(pos, tdir, data.depth + 1)*Tr));
}