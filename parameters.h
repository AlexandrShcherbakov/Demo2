#ifndef DEMO2_PARAMETERS
#define DEMO2_PARAMETERS

#include "types.h"

//Scene parameters
int polygonCount;
polygon * scene;
int verticesCount;
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

//Program parameters
enum {
    SCREEN_WIDTH = 800,
    SCREEN_HEIGHT = 600,
    SHADOWMAP_EDGE = 1024,
};

#endif
