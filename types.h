#ifndef DEMO2_TYPES
#define DEMO2_TYPES

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdio.h>

///Types discription

///Type for representation of point in 3D space
    typedef struct float3 {
        float x, y, z;
    } float3;

///Type for representation of polygon in 3D space
    typedef struct polygon {
        float3 * vertices;
        float3 normal;
        int length;
    } polygon;

///Type for representation of patch in polygon
    typedef polygon patch;

//////////////////////////////////////////////////////////

///Operators for types

///float3
///Constructor
    float3 f3(float x, float y, float z);

///Substraction for two points
    float3 sub(float3 p1, float3 p2);

///Multiply point to number
    float3 mult(float3 p, float k);

///Sum for two points
    float3 sum(float3 p1, float3 p2);

///Vector multiplication for two vectors
    float3 multV(float3 p1, float3 p2);

///Length of vector
    float length(float3 p);

///Scalar multiplication for two vectors
    float multS(float3 p1, float3 p2);

///Cosinus for angle between two vectors
    float cosV(float3 p1, float3 p2);


///polygon
///Constructor
    polygon crtPolygon(int vertCount, float * vertData, float3 norm);

///Read from file
    polygon readPolygon(FILE *fl);

///Square of polygon
    float square(polygon plg);

///Normal vector for polygon
    float3 normal(polygon p);

///Random point in square
    float3 randomPointInSquare(polygon p);

///Reflect point at center of polygon
    float3 polarizePointInPolygon(polygon pl, float3 pnt);

///Check if point in polygon
    int inPolygon(polygon pl, float3 p);

///Comparator for polygons
    int compar (const void* p1, const void* p2);

///Distance between point and area
    float distance(polygon pl, float3 p);

///Check if ray has mutual point with polygon
    int checkIntersection(polygon pl, float3 p1, float3 p2);

///Center of weightes
	float3 center(patch p);

///Random point in polygon
	float3 randomPoint(patch p);

#endif