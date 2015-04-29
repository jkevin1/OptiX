#include "Utilities.h"
#include "SphereLight.h"

#define T_MIN 1e-4
#define T_MAX 1e+4
#define DL_CHANCE 0.5f

struct RayData {
	float3 result;
	unsigned depth;
	unsigned* seed;
	int id;
};

// Buffers to store different components of the image
rtBuffer<float3, 2> accum;	// Accumulation buffer for color
rtBuffer<uchar4, 2> image;	// Buffer for image, set to accum / frame, w is object id

// Variables describing the viewport and camera
rtDeclareVariable(uint2, dim, rtLaunchDim, );
rtDeclareVariable(uint2, pixel, rtLaunchIndex, );
rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3, U, , );
rtDeclareVariable(float3, V, , );
rtDeclareVariable(float3, W, , );

// Variables describing the scene
rtBuffer<PotentialLight> lights;
rtDeclareVariable(rtObject, objects, , );
rtDeclareVariable(int, samples, , );
rtDeclareVariable(int, maxDepth, , );
rtDeclareVariable(int, frame, , );

RT_PROGRAM void pathTrace() {
	float2 invDim2 = 2.0 / make_float2(dim);
	float3 acc = make_float3(0.0f, 0.0f, 0.0f);
	unsigned seed = createSeed(pixel.y*dim.x + pixel.x, frame);
	RayData new_prd;
	new_prd.seed = &seed;
	Ray ray = make_Ray(eye, acc, 0, T_MIN, T_MAX);	// acc is 0,0,0
	for (int sample = 0; sample < samples; sample++) {
		float2 point = (make_float2(pixel) + make_float2(rand(seed), rand(seed))) * invDim2 - 1.0f;	// Random pixel in scene
		ray.direction = normalize(point.x * U + point.y * V + W);
		new_prd.depth = 0;
		rtTrace(objects, ray, new_prd);
		float luma = luminance(new_prd.result);// *0.5f;
		acc += new_prd.result / (1.0f + luma);
	}
	accum[pixel] = (frame <= 1) ? acc / float(samples) : accum[pixel] + acc / float(samples);
	float3 result = correct(clamp(accum[pixel] / float(frame), 0.0f, 1.0f));
	image[pixel] = make_uchar4( (unsigned char)(result.x * 255 + 0.5f),
								(unsigned char)(result.y * 255 + 0.5f),
								(unsigned char)(result.z * 255 + 0.5f),
								(unsigned char)new_prd.id);
}

RT_PROGRAM void exception() {
	const unsigned int code = rtGetExceptionCode();
	rtPrintf("Caught exception 0x%X at launch index (%d, %d)\n", code, pixel.x, pixel.y);
	image[pixel] = make_uchar4(255, 0, 255, 0);
}

// Variables describing the current ray intersection
rtDeclareVariable(float, t, rtIntersectionDistance, );
rtDeclareVariable(RayData, current_prd, rtPayload, );
rtDeclareVariable(Ray, current_ray, rtCurrentRay, );
rtDeclareVariable(float3, n, attribute normal, );
rtDeclareVariable(float3, e, attribute emission, );
rtDeclareVariable(float3, f, attribute color, );
rtDeclareVariable(int, id, attribute ID, );

RT_PROGRAM void miss() {
	current_prd.result = make_float3(0.0f, 0.0f, 0.0f);
	current_prd.id = 0;
}

RT_PROGRAM void diffuse() {
	if (current_prd.depth < maxDepth && length(e) < 1) {
		float3 pos = current_ray.origin + t * current_ray.direction;
		float3 dir = cosineSample(n, *current_prd.seed);

		//Randomized direct component
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
				// Weight proportionally to area of  light over hemisphere * BRDF
				weight = 0.5f * (lights[i].radius * lights[i].radius) / dot(ln, ln) / DL_CHANCE;	
			}
		}	

		// Recurse
		Ray ray = make_Ray(pos, normalize(dir), 0, T_MIN, T_MAX);
		RayData new_prd;
		new_prd.depth = current_prd.depth + 1;
		new_prd.seed = current_prd.seed;
		rtTrace(objects, ray, new_prd);

		current_prd.result = e + f * (weight * new_prd.result);
		if (current_prd.depth == 0) current_prd.id = id;
		return;
	}
	current_prd.result = e;
	current_prd.id = id;
}

RT_PROGRAM void reflect() {
	if (current_prd.depth < maxDepth && length(e) < 1) {
		float3 pos = current_ray.origin + t * current_ray.direction;
		float3 dir = reflect(current_ray.direction, n);

		Ray ray = make_Ray(pos, normalize(dir), 0, T_MIN, T_MAX);
		RayData new_prd;
		new_prd.depth = current_prd.depth + 1;
		new_prd.seed = current_prd.seed;
		rtTrace(objects, ray, new_prd);

		// Apply the Rendering Equation here.
		current_prd.result = e + (f * new_prd.result);
		if (current_prd.depth == 0) current_prd.id = id;
		return;
	}
	current_prd.result = e;
}

RT_PROGRAM void refract() {
	if (current_prd.depth < maxDepth && length(e) < 1) {
		float3 pos = current_ray.origin + t * current_ray.direction;
		float3 dir;
		if (!refract(dir, current_ray.direction, n, 1.5f)) {
			dir = reflect(current_ray.direction, -n);
		}

		Ray ray = make_Ray(pos, normalize(dir), 0, T_MIN, T_MAX);
		RayData new_prd;
		new_prd.depth = current_prd.depth + 1;
		new_prd.seed = current_prd.seed;
		rtTrace(objects, ray, new_prd);

		// Apply the Rendering Equation here.
		current_prd.result = e + (f * new_prd.result);
		if (current_prd.depth == 0) current_prd.id = id;
		return;
	}
	current_prd.result = e;
	current_prd.id = id;
}