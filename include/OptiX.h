#pragma once

/*	
 *	Includes necessary headers for optix
 *	Defines some constant values for the program
 *	Declares some helper functions for optix
 */

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

using namespace optix;

Context createContext(unsigned num_ray_types, unsigned stack_size = 2048u);

Material createPhongMaterial(Context context, float3 Ks, float3 Ka, float3 Kd, float exp, float3 ref, float opacity, float index = 1.0f);

Geometry createGeometry(Context context, const char* ptx);	//uses intersect and bounds as function names

TextureSampler loadTexture(Context context, const char* filename); //png images