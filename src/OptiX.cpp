#include "OptiX.h"
#include "Common.h"

using namespace optix;

Context createContext(unsigned num_ray_types, unsigned stack_size) {
	Context instance = Context::create();
	instance->setEntryPointCount(1);
	instance->setRayTypeCount(num_ray_types);
	instance->setStackSize(stack_size);
	return instance;
}

Material createPhongMaterial(Context context, float3 Ks, float3 Ka, float3 Kd, float exp, float3 ref) {
	Material material = context->createMaterial();

	std::string file = "phong.cu.ptx";

	//Register radiance program for the radiance ray type
    material->setClosestHitProgram(RADIANCE_RAY, context->createProgramFromPTXFile(file, "radiance"));

	//Register shadow program for the shadow ray type
    material->setAnyHitProgram(SHADOW_RAY, context->createProgramFromPTXFile(file, "shadow"));

    //Set the material properties
    material["Ks"]->setFloat(Ks.x, Ks.y, Ks.z);
    material["Ka"]->setFloat(Ka.x, Ka.y, Ka.z);
    material["Kd"]->setFloat(Kd.x, Kd.y, Kd.z);
    material["e"]->setFloat(exp);
    material["ref"]->setFloat(ref.x, ref.y, ref.z);

	return material;
}

Geometry createGeometry(Context context, const char* ptx) {
	Geometry geom = context->createGeometry();
	geom->setPrimitiveCount(1u);
	geom->setBoundingBoxProgram(context->createProgramFromPTXFile(ptx, "bounds"));
	geom->setIntersectionProgram(context->createProgramFromPTXFile(ptx, "intersect"));
	return geom;
}