#ifndef DEMO2_CLELEMENTS
#define DEMO2_CLELEMENTS


#include <stdio.h>
#include "types.h"
#include "parameters.h"

#include "CL/clew.h"
#include "CL/cl_gl.h"

#include "SDL/SDL.h"

//Constants
///////////////////////////////////////////////////////

enum {
	LIGHT_PARAMETERS_COUNT = 8,
	OPTIMIZE_CONST = 16,
};

//Global variables
///////////////////////////////////////////////////////

//Command queue
cl_command_queue clCommand;

//Program
cl_command_queue clProg;
cl_program program;

//Context
cl_context clContext;

//Array of indirect illumination
float * indirectPart;

//Parameters of light source: inner angle (float), outter angle (float), light position (float3), light direction (float3)
float lightParams[LIGHT_PARAMETERS_COUNT];

//Internal buffers for form-factors
cl_mem intCLFormFactor;
cl_mem halfCLFormFactor;

//Matrix for view from light position
cl_mem intCLLightMatrix;

//Parameters of spotlight source: Inner angle(float), Outter angle(float), Light position (float3), light direction (float3)
cl_mem intCLLightParameters;

//Points of patches
cl_mem intCLPatchesPoints;
cl_mem halfCLPatchesPoints;

//Radiosity buffers
cl_mem halfCLIncident;
cl_mem halfCLExcident;
cl_mem halfCLReflection;

cl_mem halfCLPreIncident;
cl_mem halfCLCenterIncident;

//Shadow map
cl_mem clShadowMap;

//Indices of patches
cl_mem intCLIndices;

//Reflections of materials
cl_mem intCLMatReflection;

//Buffer for hammersley distribution
cl_float2 hammersleyDist[LIGHT_COUNT_ITERATIONS];
cl_mem hammersleyCLBuf;

///Kernels
cl_kernel computeLightEmission;
cl_kernel convertROFloatToHalf;
cl_kernel sendRays;
cl_kernel sendRaysV3;
cl_kernel sendRaysV4;
cl_kernel sendRaysV5;
cl_kernel sendRaysV6;
cl_kernel sendRaysV7;
cl_kernel reduceIncident;
cl_kernel reduceIncidentV2;
cl_kernel reduceIncidentV3;
cl_kernel reduceIncidentV4;
cl_kernel replaceIncident;
cl_kernel interpolation;

//Error code
int cl_err;


//Debug defines
//////////////////////////////////////////////////////
#define CHECK_CL(call) checkCLFun((call), __FILE__, __LINE__);


//Check functions
//////////////////////////////////////////////////////
const char * getOpenCLErrorString(cl_int err);

void checkCLFun(cl_int cErr, char* file, int line);

//Other functions
///////////////////////////////////////////////////////

//Load GL functions
///////////////////////////////////////////////////////


#endif
