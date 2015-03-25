#include "OptiX.h"
#include "Common.h"
#include "lodepng.h"

using namespace optix;

Context createContext(unsigned num_ray_types, unsigned stack_size) {
	Context instance = Context::create();
	instance->setEntryPointCount(1);
	instance->setRayTypeCount(num_ray_types);
	instance->setStackSize(stack_size);
	return instance;
}

Material createPhongMaterial(Context context, float3 Ks, float3 Ka, float3 Kd, float exp, float3 ref, float opacity, float index) {
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
	material["opacity"]->setFloat(opacity);
	material["index"]->setFloat(index);

	return material;
}

Geometry createGeometry(Context context, const char* ptx) {
	Geometry geom = context->createGeometry();
	geom->setPrimitiveCount(1u);
	geom->setBoundingBoxProgram(context->createProgramFromPTXFile(ptx, "bounds"));
	geom->setIntersectionProgram(context->createProgramFromPTXFile(ptx, "intersect"));
	return geom;
}

TextureSampler loadTexture(Context context, const char* filename) {
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filename);

	TextureSampler sampler = context->createTextureSampler();
	sampler->setWrapMode(0, RT_WRAP_REPEAT);
	sampler->setWrapMode(1, RT_WRAP_REPEAT);
	sampler->setWrapMode(2, RT_WRAP_REPEAT);
	sampler->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
	sampler->setReadMode(RT_TEXTURE_READ_NORMALIZED_FLOAT);
	sampler->setMaxAnisotropy(1.0f);
	sampler->setMipLevelCount(1u);
	sampler->setArraySize(1u);
	
	if (error) {	//Return solid fuschia texture
		printf("Error loading %s: %s\n", filename, lodepng_error_text(error));
		optix::Buffer buffer = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_BYTE4, 1u, 1u);
		unsigned char* data = static_cast<unsigned char*>(buffer->map());
		data[0] = 255;
		data[1] = 0;
		data[2] = 255;
		data[3] = 255;
		buffer->unmap();
		sampler->setBuffer(0u, 0u, buffer);
		sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
		return sampler;
	}

	optix::Buffer buffer = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_BYTE4, width, height);
	unsigned char* data = static_cast<unsigned char*>(buffer->map());
	
	for (unsigned i = 0; i < width; ++i) {
		for (unsigned j = 0; j < height; ++j) {
			//unrolled, why not
			unsigned bindex = (j*width + i) * 4;
			unsigned iindex = ((height-j-1)*width + i) * 4;
			data[bindex + 0] = image[iindex + 0];
			data[bindex + 1] = image[iindex + 1];
			data[bindex + 2] = image[iindex + 2];
			data[bindex + 3] = image[iindex + 3];
		}
	}

	buffer->unmap();
	sampler->setBuffer(0u, 0u, buffer);
	sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
	return sampler;
}