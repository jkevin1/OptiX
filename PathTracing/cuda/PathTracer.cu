#include "Utilities.h"
#include "SphereLight.h"

#define T_MIN 1e-4
#define T_MAX 1e+4

struct RayData {
	float3 result;
	unsigned depth;
	unsigned* seed;
};

rtBuffer<float4, 2> image;
rtDeclareVariable(uint2, dim, rtLaunchDim, );
rtDeclareVariable(uint2, pixel, rtLaunchIndex, );
rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3, U, , );
rtDeclareVariable(float3, V, , );
rtDeclareVariable(float3, W, , );
rtDeclareVariable(rtObject, objects, , );
rtDeclareVariable(int, NUM_SAMPLES, , );
rtDeclareVariable(int, MAX_DEPTH, , );
rtDeclareVariable(int, frame, , );

rtBuffer<SphereLight> lights;
rtDeclareVariable(float, t, rtIntersectionDistance, );
rtDeclareVariable(RayData, current_prd, rtPayload, );
rtDeclareVariable(Ray, current_ray, rtCurrentRay, );
rtDeclareVariable(float3, n, attribute normal, );
rtDeclareVariable(float3, e, attribute emission, );
rtDeclareVariable(float3, f, attribute color, );

RT_PROGRAM void pathTrace() {
	uint2 dim = image.size();
	float2 invDim2 = 2.0 / make_float2(dim);
	float3 acc = make_float3(0.0f, 0.0f, 0.0f);
	for (int sample = 0; sample < NUM_SAMPLES; sample++) {
		unsigned seed = createSeed(pixel.y*pixel.y*pixel.x, frame * NUM_SAMPLES + sample);
		float2 point = (make_float2(pixel) + make_float2(rand(seed), rand(seed))) * invDim2 - 1.0f;	// Random pixel in scene
		float3 dir = normalize(point.x * U + point.y * V + W);
		Ray ray = make_Ray(eye, dir, 0, T_MIN, T_MAX);
		RayData new_prd;
		new_prd.depth = 0;
		new_prd.seed = &seed;
		rtTrace(objects, ray, new_prd);
		acc += new_prd.result;
	}
	acc *= 1.0f / float(NUM_SAMPLES);
	if (frame == 0) image[pixel] = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
	image[pixel] += make_float4(acc, 1.0f);
}

RT_PROGRAM void exception() {
	const unsigned int code = rtGetExceptionCode();
	rtPrintf("Caught exception 0x%X at launch index (%d, %d)\n", code, pixel.x, pixel.y);
	image[pixel] = make_float4(1.0f, 0.0f, 1.0f, 1.0f);
}

RT_PROGRAM void miss() {
	current_prd.result = make_float3(0.0f, 0.0f, 0.0f);
}

#define DL_CHANCE 0.2f

RT_PROGRAM void diffuse() {
	if (current_prd.depth < MAX_DEPTH && length(e) < 1) {
		float3 pos = current_ray.origin + t * current_ray.direction;
		/*/ TODO: Direct component
		float3 dl = make_float3(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < lights.size(); i++) {
			float3 ln = pos - lights[i].position;
			float3 sample = lights[i].position + lights[i].radius * cosineSample(normalize(ln), *current_prd.seed);
			float3 dir = sample - pos;
			float ndl = dot(n, dir);
			if (ndl > 0) {
				Ray ray = make_Ray(pos, normalize(dir), 0, T_MIN, length(ln));
				RayData new_prd;
				new_prd.depth = current_prd.depth + 1;
				new_prd.seed = current_prd.seed;
				rtTrace(objects, ray, new_prd);
				dl += new_prd.result * ndl / M_PI;
			}
		}	*/
		// Indirect component:
		// Pick a random direction from here and keep going.

		//Randomized direct component
		float3 dir = cosineSample(n, *current_prd.seed);
		float weight = 1.0;
		float r = rand(*current_prd.seed);
		if (r < DL_CHANCE) {
			int i = (int)(lights.size() * r / DL_CHANCE);
			float3 ln = pos - lights[i].position;
			float3 point = normalize(cosineSample(normalize(ln), *current_prd.seed));
			float3 sample = lights[i].position + lights[i].radius * point;
			float ndl = dot(n, sample - pos);
			if (ndl > 0) {
				dir = sample - pos;
				weight = ndl * M_1_PIf * 0.5f * (lights[i].radius * lights[i].radius) / dot(ln, ln);	//proportional area of  light over hemisphere * BRDF
			}
		}	

		Ray ray = make_Ray(pos, normalize(dir), 0, T_MIN, T_MAX);
		RayData new_prd;
		new_prd.depth = current_prd.depth + 1;
		new_prd.seed = current_prd.seed;
		rtTrace(objects, ray, new_prd);

		// Apply the Rendering Equation here.
		current_prd.result = e + f * (weight * new_prd.result);
		return;
	}
	current_prd.result = e;
}