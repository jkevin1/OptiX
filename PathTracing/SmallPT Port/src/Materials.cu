#include "RayData.h"
#include "SphereLight.h"
#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>

#define M_PI 3.141592653589793

using namespace optix;

rtBuffer<SphereLight> lights;
rtDeclareVariable(float3, f, , );
rtDeclareVariable(float3, e, , );

rtDeclareVariable(Ray, r, rtCurrentRay, );
rtDeclareVariable(float, t, rtIntersectionDistance, );
rtDeclareVariable(RayData, data, rtPayload, );
rtDeclareVariable(rtObject, objects, , );
rtDeclareVariable(int, maxDepth, , );

rtDeclareVariable(float3, n, attribute normal, );
rtDeclareVariable(int, id, attribute ID, );

static __device__ __inline__ float3 radiance(float3 o, float3 d, int depth, int E = 1) {
	RayData payload;
	payload.E = E;
	payload.depth = depth;
	payload.seed = data.seed;
	optix::Ray ray = optix::make_Ray(o, d, 0, DEFAULT_MIN, DEFAULT_MAX);
	rtTrace(objects, ray, payload);
	return payload.result;
}

RT_PROGRAM void diffuse() {		// Ideal diffuse reflection
	data.id = id;
	if (data.depth >= maxDepth) {
		data.result = e * data.E;
		return;
	}
	float3 pos = r.origin + r.direction*t;
	float3 nl = dot(n, r.direction) < 0 ? n : -n;
	float p = fmaxf(f); // max reflectance

	float r1 = 2 * M_PI*randFloat(data.seed);
	float r2 = randFloat(data.seed), r2s = sqrt(r2);
	float3 w = nl;
	float3 u = normalize(cross((fabs(w.x) > .1 ? make_float3(0.0f, 1.0f, 0.0f) : make_float3(1.0f, 0.0f, 0.0f)), w));
	float3 v = cross(w, u);
	float3 d = normalize(u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1 - r2));

	// Possibly optional line?
//	if (++depth>5) if (randFloat()<p) f = f*(1 / p); else return obj.e;
	float3 direct = make_float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < lights.size(); i++) {
		const SphereLight& s = lights[i];
		float3 sw = s.p - pos, su = normalize(cross((fabs(sw.x) > .1 ? make_float3(0, 1, 0) : make_float3(1, 0, 0)), sw)), sv = cross(sw, su);
		float cos_a_max = sqrt(1 - s.r*s.r / dot(pos - s.p, pos - s.p));
		float eps1 = randFloat(data.seed), eps2 = randFloat(data.seed);
		float cos_a = 1 - eps1 + eps2*cos_a_max;
		float sin_a = sqrt(1 - cos_a*cos_a);
		float phi = 2 * M_PI * eps2;
		float3 l = normalize(su*cos(phi)*sin_a + sv*sin(phi)*sin_a + sw*cos_a);
		Ray ray = make_Ray(pos, l, 0, DEFAULT_MIN, DEFAULT_MAX);
		RayData payload;
		payload.id = s.id - 1;	// TODO: update to shadow ray to avoid shading
		rtTrace(objects, ray, payload);
		if (payload.id == s.id) {
			float omega = 2 * M_PI * (1 - cos_a_max);
			direct += f * (s.e * dot(l, n) * omega) * M_1_PIf;
		}
	}
	
	data.result = e*data.E + direct + (f * radiance(pos, d, data.depth + 1, 0));
}

RT_PROGRAM void specular() {	// Ideal specular reflection
	data.id = id;
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
	data.id = id;
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
		return;
	}
	float3 tdir = normalize(r.direction*nnt - n*((into ? 1 : -1)*(ddn*nnt + sqrt(cos2t))));
	float a = nt - nc, b = nt + nc, R0 = a*a / (b*b), c = 1 - (into ? -ddn : dot(tdir, n));
	float Re = R0 + (1 - R0)*c*c*c*c*c, Tr = 1 - Re, P = .25 + .5*Re, RP = Re / P, TP = Tr / (1 - P);
	data.result = e + (f * (data.depth > 2 ? (randFloat(data.seed) < P ?   // Russian roulette
		radiance(pos, reflDir, data.depth + 1)*RP : radiance(pos, tdir, data.depth + 1)*TP) :
		radiance(pos, reflDir, data.depth + 1)*Re + radiance(pos, tdir, data.depth + 1)*Tr));
}