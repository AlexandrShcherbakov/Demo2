#ifndef DEMO2_TYPES
#define DEMO2_TYPES

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdio.h>

///Types discription

typedef unsigned uint;

///Type for representation of point in 3D space
    typedef struct vec3 {
        float x, y, z;
    } vec3;

///Type for representation of point in 4D space
    typedef struct vec4 {
        float x, y, z, w;
    } vec4;

///Type for material
    typedef struct Material {
        vec4 ambient;
        vec4 diffuse;
        vec4 specular;
        float shininess;
    } Material;


///Type for representation of polygon in 3D space
    typedef struct polygon {
        vec3 * vertices;
        vec3 normal;
        int length;
        Material mat;
    } polygon;


///Type for representation of patch in polygon
    typedef polygon patch;


///Patched polygon
    typedef struct patched_polygon {
        patch * patches;
        int axis1, axis2;
    } patched_polygon;


///Type for spotlight source
    typedef struct LightSource {
        vec4 ambient;
        vec4 diffuse;
        vec4 specular;
        vec3 spotDirection;
        float spotCosCutoff;
    } LightSource;


///Type for spotlight source
    typedef struct SpotLight {
        vec4 position;
        vec3 intensities;
        float attenuation;
        float ambientCoefficient;
        float coneAngle;
        vec3 coneDirection;
    } SpotLight;

//////////////////////////////////////////////////////////

///Operators for types

///float3
///Constructor
    vec3 v3(float x, float y, float z);

///Substraction for two points
    vec3 sub(vec3 p1, vec3 p2);

///Multiply point to number
    vec3 mult(vec3 p, float k);

///Sum for two points
    vec3 sum(vec3 p1, vec3 p2);

///Vector multiplication for two vectors
    vec3 multV(vec3 p1, vec3 p2);

///Length of vector
    float length(vec3 p);

///Scalar multiplication for two vectors
    float multS(vec3 p1, vec3 p2);

///Cosinus for angle between two vectors
    float cosV(vec3 p1, vec3 p2);

///Normalized vector
    vec3 normalize(vec3 v);

///float4
///Constructor
    vec4 v4(float x, float y, float z, float w);


///polygon
///Constructor
    polygon crtPolygon(int vertCount, float * vertData, vec3 norm, Material mat);

///Read from file
    polygon readPolygon(FILE *fl);

///Create patched polygon from polygon
    patched_polygon SplitPolygonToPatches(polygon poly, int patchCount);

///Square of polygon
    float square(polygon plg);

///Normal vector for polygon
    vec3 normal(polygon p);

///Random point in square
    vec3 randomPointInSquare(polygon p);

///Reflect point at center of polygon
    vec3 polarizePointInPolygon(polygon pl, vec3 pnt);

///Check if point in polygon
    int inPolygon(polygon pl, vec3 p);

///Comparator for polygons
    int compar (const void* p1, const void* p2);

///Distance between point and area
    float distance(polygon pl, vec3 p);

///Check if ray has mutual point with polygon
    int checkIntersection(polygon pl, vec3 p1, vec3 p2);

///Center of weightes
	vec3 center(patch p);

///Random point in polygon
	vec3 randomPoint(patch p);

#endif
