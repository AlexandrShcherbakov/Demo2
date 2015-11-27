#ifndef DEMO2_PARAMETERS
#define DEMO2_PARAMETERS

#include "types.h"

//Scene parameters
int polygonCount;
polygon * scene;
int verticesCount;
vec3 spotLightDirection;
vec3 rotate;

//Program parameters
enum {
    SCREEN_WIDTH = 800,
    SCREEN_HEIGHT = 600,
};

#endif
