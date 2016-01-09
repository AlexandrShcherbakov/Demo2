#include "types.h"
#include <stdbool.h>
#include <time.h>

#ifndef DEMO2_PARAMETERS
#define DEMO2_PARAMETERS

//Scene parameters
int polygonCount;
polygon * scene;
patched_polygon * pt_scene;
int verticesCount;
int ptVerticesCount;
vec3 spotLightDirection;
vec3 rotate;
vec3 spotLightPosition;
vec3 viewPoint;
vec3 viewDirection;
vec3 CamViewerRotation;
float viewAngle;
float spotAngIn;
float spotAngOut;
bool actualShadowMap;
float *formFactors;
int patchCount;
int PATCH_COUNT;
clock_t timer;
float indirectBright;
float directBright;

//Program parameters
enum {
    SCREEN_WIDTH = 800,
    SCREEN_HEIGHT = 600,
    SHADOWMAP_EDGE = 1024,
    MONTE_KARLO_ITERATIONS_COUNT = 20,
    LIGHT_COUNT_ITERATIONS = 100,
};

#endif
