#include <optix.h>
#include "sutil.h"
#include "commonStructs.h"
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <time.h>

#define NUM_SPHERES 16

/*
This program is intended simply to learn how to create an OptiX program
It uses the sutil library, which is supplied by the OptiX SDK
It also uses the commonStructs header, also from the OptiX SDK
*/

const unsigned width = 1024u;
const unsigned height = 768u;

void createContext(RTcontext* context, RTbuffer* output);
void createSphere(RTcontext context, RTgeometry* sphere);
void createMaterial(RTcontext context, RTmaterial* material);
void createInstances(RTcontext context, RTgeometry sphere, RTmaterial material);

int main(int argc, char* argv[]) {
	RTcontext           context;
    RTbuffer            output;
    RTgeometry          sphere;
    RTmaterial          material;
	
	srand(time(NULL));

	RT_CHECK_ERROR_NO_CONTEXT(sutilInitGlut(&argc, argv));
	createContext(&context, &output);
    createSphere(context, &sphere);
    createMaterial(context, &material);
    createInstances(context, sphere, material);

	RT_CHECK_ERROR(rtContextValidate(context));
    RT_CHECK_ERROR(rtContextCompile(context));
    RT_CHECK_ERROR(rtContextLaunch2D(context, 0, width, height));
	RT_CHECK_ERROR(sutilDisplayBufferInGlutWindow(argv[0], output));
    RT_CHECK_ERROR(rtContextDestroy(context));
	system("pause");
	return 0;
}

void createContext(RTcontext* context, RTbuffer* output) {
	RTprogram  ray_gen_program;
    RTprogram  miss_program;
    RTbuffer   light_buffer_obj;
    RTvariable output_buffer;
    RTvariable light_buffer;
    RTvariable radiance_ray_type;
    RTvariable shadow_ray_type;
    RTvariable epsilon;
    RTvariable max_depth;

    BasicLight light;
    void* light_buffer_data;

    /* variables for ray gen program */
    RTvariable eye;
    RTvariable U;
    RTvariable V;
    RTvariable W;

    /* variables for the miss program */
    RTvariable color;

    //Initialize the context
    RT_CHECK_ERROR2(rtContextCreate(context));
	//There are standard rays and shadow rays
    RT_CHECK_ERROR2(rtContextSetRayTypeCount(*context, 2));
	//Only one entry point
    RT_CHECK_ERROR2(rtContextSetEntryPointCount(*context, 1));

	//Declare all the variables, copied from sample 2
    RT_CHECK_ERROR2(rtContextDeclareVariable(*context, "output_buffer", &output_buffer));
    RT_CHECK_ERROR2(rtContextDeclareVariable(*context, "lights", &light_buffer));
    RT_CHECK_ERROR2(rtContextDeclareVariable(*context, "max_depth", &max_depth));
    RT_CHECK_ERROR2(rtContextDeclareVariable(*context, "radiance_ray_type", &radiance_ray_type));
    RT_CHECK_ERROR2(rtContextDeclareVariable(*context, "shadow_ray_type", &shadow_ray_type));
    RT_CHECK_ERROR2(rtContextDeclareVariable(*context, "scene_epsilon", &epsilon));

	//10 maximum reflections
    RT_CHECK_ERROR2(rtVariableSet1i(max_depth, 10u));
    //Standard ray has id 0
	RT_CHECK_ERROR2(rtVariableSet1ui(radiance_ray_type, 0u));
    //Shadow ray has id 1
	RT_CHECK_ERROR2(rtVariableSet1ui(shadow_ray_type, 1u));
    RT_CHECK_ERROR2(rtVariableSet1f(epsilon, 1.e-4f));

    //Create the output buffer
    RT_CHECK_ERROR2(rtBufferCreate(*context, RT_BUFFER_OUTPUT, output));
    RT_CHECK_ERROR2(rtBufferSetFormat(*output, RT_FORMAT_UNSIGNED_BYTE4));
    RT_CHECK_ERROR2(rtBufferSetSize2D(*output, width, height));
    RT_CHECK_ERROR2(rtVariableSetObject(output_buffer, *output));

    //Initialize the light
    light.color.x = 0.8f;
    light.color.y = 0.8f;
    light.color.z = 0.8f;
    light.pos.x   = 10.0f;
    light.pos.y   = 1.0f;
    light.pos.z   = 10.0f;
    light.casts_shadow = 1;
    light.padding      = 0u;
    RT_CHECK_ERROR2(rtBufferCreate(*context, RT_BUFFER_INPUT, &light_buffer_obj));
    RT_CHECK_ERROR2(rtBufferSetFormat(light_buffer_obj, RT_FORMAT_USER));
    RT_CHECK_ERROR2(rtBufferSetElementSize(light_buffer_obj, sizeof(BasicLight)));
    RT_CHECK_ERROR2(rtBufferSetSize1D(light_buffer_obj, 1));
    RT_CHECK_ERROR2(rtBufferMap(light_buffer_obj, &light_buffer_data));
    ((BasicLight*)light_buffer_data)[0] = light;
    RT_CHECK_ERROR2(rtBufferUnmap(light_buffer_obj));
    RT_CHECK_ERROR2(rtVariableSetObject(light_buffer, light_buffer_obj));

    /* Ray generation program */
    RT_CHECK_ERROR2(rtProgramCreateFromPTXFile(*context, "camera.cu.ptx", "genRay", &ray_gen_program));
    RT_CHECK_ERROR2(rtProgramDeclareVariable(ray_gen_program, "eye", &eye));
    RT_CHECK_ERROR2(rtProgramDeclareVariable(ray_gen_program, "U", &U));
    RT_CHECK_ERROR2(rtProgramDeclareVariable(ray_gen_program, "V", &V));
    RT_CHECK_ERROR2(rtProgramDeclareVariable(ray_gen_program, "W", &W));
    RT_CHECK_ERROR2(rtVariableSet3f(eye, 0.0f, 0.0f, 10.0f));
    RT_CHECK_ERROR2(rtVariableSet3f(U, 1.0f, 0.0f, 0.0f));
    RT_CHECK_ERROR2(rtVariableSet3f(V, 0.0f, 1.0f, 0.0f));
    RT_CHECK_ERROR2(rtVariableSet3f(W, 0.0f, 0.0f, -1.0f));
    RT_CHECK_ERROR2(rtContextSetRayGenerationProgram(*context, 0, ray_gen_program));

    //Dark blue-gray background color
    RT_CHECK_ERROR2(rtProgramCreateFromPTXFile(*context, "camera.cu.ptx", "miss", &miss_program));
    RT_CHECK_ERROR2(rtProgramDeclareVariable(miss_program, "bg_color" , &color));
    RT_CHECK_ERROR2(rtVariableSet3f(color, 0.2f, 0.2f, 0.5f));
    RT_CHECK_ERROR2(rtContextSetMissProgram(*context, 0, miss_program));
}

inline float randFloat(float min, float max) {
	double val = float(rand()) / float(RAND_MAX);
	return min + (val * (max - min));
}

void createSphere(RTcontext context, RTgeometry* sphere) {
	RTprogram intersection_program;
	RTprogram bounding_box_program;
	RTvariable s;
	float sphere_definition[4] =  {0, 0, 0, 1.0f};

	RT_CHECK_ERROR(rtGeometryCreate(context, sphere));
	RT_CHECK_ERROR(rtGeometrySetPrimitiveCount(*sphere, 1u));

	RT_CHECK_ERROR(rtProgramCreateFromPTXFile(context, "sphere.cu.ptx", "bounds", &bounding_box_program));
	RT_CHECK_ERROR(rtGeometrySetBoundingBoxProgram(*sphere, bounding_box_program));
	RT_CHECK_ERROR(rtProgramCreateFromPTXFile(context, "sphere.cu.ptx", "intersect", &intersection_program));
	RT_CHECK_ERROR(rtGeometrySetIntersectionProgram(*sphere, intersection_program));

	RT_CHECK_ERROR(rtGeometryDeclareVariable(*sphere, "sphere" , &s));
	RT_CHECK_ERROR(rtVariableSet4fv(s, &sphere_definition[0]));
}

void createMaterial(RTcontext context, RTmaterial* material) {
	//Create material
	RT_CHECK_ERROR(rtMaterialCreate(context, material));
	
	//Set up material properties
	RTvariable Ks;
	RTvariable exp;
	RTvariable Ka;
	RTvariable Kd;
	RTvariable reflectivity;
	rtMaterialDeclareVariable(*material, "Ks", &Ks);
	rtMaterialDeclareVariable(*material, "exp", &exp);
	rtMaterialDeclareVariable(*material, "Ka", &Ka);
	rtMaterialDeclareVariable(*material, "Kd", &Kd);
	rtMaterialDeclareVariable(*material, "reflectivity", &reflectivity);
	rtVariableSet3f(Ks, 0.4f, 0.4f, 0.4f);
	rtVariableSet1f(exp, 100.0f);
	rtVariableSet3f(Ka, 0.1f, 0.1f, 0.1f);
	rtVariableSet3f(Kd, 1.0f, 1.0f, 1.0f);
	rtVariableSet3f(reflectivity, 0.2f, 0.2f, 0.2f);

	//Leaving any hit null
	RTprogram closest_hit_program;
	RT_CHECK_ERROR(rtProgramCreateFromPTXFile(context, "phong.cu.ptx", "closest_hit_radiance", &closest_hit_program));
	RT_CHECK_ERROR(rtMaterialSetClosestHitProgram(*material, 0, closest_hit_program));
	
	//Leaving closest hit null
	RTprogram any_hit_program;
	RT_CHECK_ERROR(rtProgramCreateFromPTXFile(context, "phong.cu.ptx", "any_hit_shadow", &any_hit_program));
	RT_CHECK_ERROR(rtMaterialSetAnyHitProgram(*material, 1, any_hit_program));
}


void createInstances(RTcontext context, RTgeometry sphere, RTmaterial material) {
    RTtransform     transforms[NUM_SPHERES];
    RTgroup         top_level_group;
    RTvariable      top_object;
    RTvariable      top_shadower;
    RTacceleration  top_level_acceleration;
    RTvariable kd;
    
    int i;

    float m[16];
    m[ 0] = 1.0f;  m[ 1] = 0.0f;  m[ 2] = 0.0f;  m[ 3] = 0.0f;
    m[ 4] = 0.0f;  m[ 5] = 1.0f;  m[ 6] = 0.0f;  m[ 7] = 0.0f;
    m[ 8] = 0.0f;  m[ 9] = 0.0f;  m[10] = 1.0f;  m[11] = 0.0f;
    m[12] = 0.0f;  m[13] = 0.0f;  m[14] = 0.0f;  m[15] = 1.0f;

    for (i = 0; i < NUM_SPHERES; ++i) {
        RTgeometrygroup geometrygroup;
        RTgeometryinstance instance;
        RTacceleration acceleration;

        //Create a new sphere instance
        RT_CHECK_ERROR(rtGeometryInstanceCreate(context, &instance));
        RT_CHECK_ERROR(rtGeometryInstanceSetGeometry(instance, sphere));
        RT_CHECK_ERROR(rtGeometryInstanceSetMaterialCount(instance, 1));
        RT_CHECK_ERROR(rtGeometryInstanceSetMaterial(instance, 0, material));

        /* create group to hold instance transform */
        RT_CHECK_ERROR(rtGeometryGroupCreate( context, &geometrygroup));
        RT_CHECK_ERROR(rtGeometryGroupSetChildCount( geometrygroup, 1));
        RT_CHECK_ERROR(rtGeometryGroupSetChild( geometrygroup, 0, instance));

        //No acceleration structure, only one element
        RT_CHECK_ERROR(rtAccelerationCreate(context,&acceleration));
        RT_CHECK_ERROR(rtAccelerationSetBuilder(acceleration,"NoAccel"));
        RT_CHECK_ERROR(rtAccelerationSetTraverser(acceleration,"NoAccel"));
        RT_CHECK_ERROR(rtGeometryGroupSetAcceleration( geometrygroup, acceleration));
        RT_CHECK_ERROR(rtAccelerationMarkDirty(acceleration));

        //Transform the current sphere to a random position
        RT_CHECK_ERROR(rtTransformCreate(context, &transforms[i]));
        RT_CHECK_ERROR(rtTransformSetChild(transforms[i], geometrygroup));
		
		//Change the diffuse color of each sphere
        RT_CHECK_ERROR(rtGeometryInstanceDeclareVariable(instance, "Kd", &kd));
		float color[3] = {0.0f, 0.0f, 0.0f};

		//Increase random component by .25 up to 8 times
		for (int i = 0; i < 5; i++) {
			int component = rand() % 4;
			if (component == 3) break;
			color[i] += 0.25f;
			if (color[i] > 1.0f) color[i] = 1.0f;
		}
        RT_CHECK_ERROR(rtVariableSet3fv(kd, color));

		//Translate
		m[ 3] = randFloat(-5.0f, 5.0f);	//x
		m[ 7] = randFloat(-5.0f, 5.0f);	//y
		m[11] = randFloat(-5.0f, 5.0f);	//z
		//Scale
		m[0] = m[5] = m[10] = randFloat(0.05f, 2.0f);
        RT_CHECK_ERROR(rtTransformSetMatrix(transforms[i], 0, m, 0));
    }

    /* Place these geometrygroups as children of the top level object */
    RT_CHECK_ERROR(rtGroupCreate(context, &top_level_group));
	RT_CHECK_ERROR(rtGroupSetChildCount(top_level_group, NUM_SPHERES));
    for (i = 0; i < NUM_SPHERES; ++i) {
      RT_CHECK_ERROR(rtGroupSetChild(top_level_group, i, transforms[i]));
    }
    RT_CHECK_ERROR(rtContextDeclareVariable(context, "top_object", &top_object));
    RT_CHECK_ERROR(rtVariableSetObject(top_object, top_level_group));
    RT_CHECK_ERROR(rtContextDeclareVariable(context, "top_shadower", &top_shadower));
    RT_CHECK_ERROR(rtVariableSetObject(top_shadower, top_level_group));
	
    //No acceleration structure, its just a couple spheres
    RT_CHECK_ERROR(rtAccelerationCreate(context, &top_level_acceleration));
    RT_CHECK_ERROR(rtAccelerationSetBuilder(top_level_acceleration,"NoAccel"));
    RT_CHECK_ERROR(rtAccelerationSetTraverser(top_level_acceleration,"NoAccel"));
    RT_CHECK_ERROR(rtGroupSetAcceleration( top_level_group, top_level_acceleration));
    RT_CHECK_ERROR( rtAccelerationMarkDirty( top_level_acceleration ) );
}