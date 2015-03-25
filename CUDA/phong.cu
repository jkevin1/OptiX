#include <optix_device.h>
#include <optixu/optixu_math_namespace.h>
#include "Common.h"

using namespace optix;

rtDeclareVariable(float3, Ka, , );
rtDeclareVariable(float3, Kd, , );
rtDeclareVariable(float3, Ks, , );
rtDeclareVariable(float3, ref, , );
rtDeclareVariable(float, e, , );
rtDeclareVariable(float, opacity, , );
rtDeclareVariable(float, index, , );
rtDeclareVariable(int, mode, , );

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, dist, rtIntersectionDistance, );
rtDeclareVariable(PerRayData_radiance, radianceData, rtPayload, );
rtDeclareVariable(rtObject, objects, , );
rtBuffer<BasicLight> lights;

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, ); 

RT_PROGRAM void radiance() {
	float3 world_snormal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 world_gnormal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometric_normal));

	float3 normal = faceforward(world_snormal, -ray.direction, world_gnormal);
	float3 location = ray.origin + dist * ray.direction;
	
	if (mode == 0) {
		float3 result = Ka;

		unsigned int num_lights = lights.size();
		for(int i = 0; i < num_lights; ++i) {
			BasicLight light = lights[i];
			float distance = optix::length(light.pos - location);
			float3 L = optix::normalize(light.pos - location);
			float dot = optix::dot(normal, L);

			// cast shadow ray
			float3 attenuation = make_float3(static_cast<float>(dot > 0.0f));
			if (dot > 0.0f && light.casts_shadow) {
				PerRayData_shadow data;
				data.attenuation = make_float3(1.0f);
				optix::Ray shadow_ray = optix::make_Ray(location, L, SHADOW_RAY, DEFAULT_MIN, distance);
				rtTrace(objects, shadow_ray, data);
				//radianceData.result = data.attenuation;
				//return;
				attenuation = data.attenuation;
			}
			//radianceData.result = make_float3(0.0f);
			//return;
			//attenuation = make_float3(1.0f);

			// If not completely shadowed, light the hit point
			if (fmaxf(attenuation) > 0.0f) {
				float3 clr = light.color * attenuation;

				result += Kd * dot * clr;

				float3 H = optix::normalize(L - ray.direction);
				float dot2 = optix::dot(normal, H);
				if(dot2 > 0.0f) {
					float power = pow(dot2, e);
					result += Ks * power * clr;
				}
			}
		}

		//Reflection
		if(fmaxf(ref) > 0.0f) {
			// ray tree attenuation
			PerRayData_radiance data;             
			data.importance = radianceData.importance * optix::luminance(ref);
			data.depth = radianceData.depth + 1;

			// reflection ray
			if(data.importance >= 0.01f && data.depth <= MAX_DEPTH) {
				float3 R = optix::reflect(ray.direction, normal);
				optix::Ray reflection = optix::make_Ray(location, R, RADIANCE_RAY, DEFAULT_MIN, RT_DEFAULT_MAX);
				rtTrace(objects, reflection, data);
				result += ref * data.result;
			}
		}

		float refracted = 1.0f - opacity;
		//Refraction
	//	if (refracted > 0.0f) {
			//TODO improve this to use fresnel function to balance reflection/refraction amounts
			PerRayData_radiance data;
			data.importance = radianceData.importance * refracted;
			data.depth = radianceData.depth + 1;

			if (data.importance >= 0.1f && data.depth <= MAX_DEPTH) {
				//calculation
				float3 R;
				if (optix::refract(R, optix::normalize(ray.direction), normal, index)) {
					optix::Ray refraction = optix::make_Ray(location, R, RADIANCE_RAY, DEFAULT_MIN, RT_DEFAULT_MAX);
					rtTrace(objects, refraction, data);
					result = (opacity * result) + (refracted * data.result);
				}
				//TODO total interal reflection
			}
	//	}
		
		// pass the color back up the tree
		radianceData.result = result;
	} else if (mode == 1) {	
		radianceData.result = 0.5f * (normal + make_float3(1.0f));
	} else if (mode == 2) {
		radianceData.result = 0.33333f * (location + make_float3(1.5f));
	}	
}

rtDeclareVariable(PerRayData_shadow, shadowData, rtPayload, );

RT_PROGRAM void shadow() {
	shadowData.attenuation = optix::make_float3(0.0f);
	rtTerminateRay();
}