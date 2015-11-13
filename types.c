#include "types.h"
///File with realization of basic operations for types discribed in types.h

float3 f3(float x, float y, float z) {
    float3 result = {x, y, z};
    return result;
}


polygon crtPolygon(int vertCount, float * vertData, float3 norm) {
    polygon result;
    result.length = vertCount;
    result.vertices = calloc(vertCount, sizeof(*result.vertices));
    for (int i = 0; i < vertCount; ++i) {
        result.vertices[i] = f3(vertData[3 * i], vertData[3 * i + 1], vertData[3 * i + 2]);
    }
    result.normal = norm;
    return result;
}


polygon readPolygon(FILE *fl) {
    polygon result;
    fscanf(fl, "%d", &(result.length));
    result.vertices = calloc(result.length, sizeof(*result.vertices));
    for (int i = 0; i < result.length; ++i) {
        fscanf(fl, "%f", &(result.vertices[i].x));
        fscanf(fl, "%f", &(result.vertices[i].y));
        fscanf(fl, "%f", &(result.vertices[i].z));
    }
    fscanf(fl, "%f", &(result.normal.x));
    fscanf(fl, "%f", &(result.normal.y));
    fscanf(fl, "%f", &(result.normal.z));
    return result;
}


inline float3 sub(float3 p1, float3 p2) {
    float3 res = {p1.x - p2.x, p1.y - p2.y, p1.z - p2.z};
    return res;
}


inline float3 mult(float3 p, float k) {
    float3 res = {p.x * k, p.y * k, p.z * k};
    return res;
}


inline float3 sum(float3 p1, float3 p2) {
    float3 res = {p1.x + p2.x, p1.y + p2.y, p1.z + p2.z};
    return res;
}


float square(polygon plg) {
	float res = 0;
	for (int i = 2; i < plg.length; ++i) {
        res += length(multV(sub(plg.vertices[i], plg.vertices[0]),
							sub(plg.vertices[i - 1], plg.vertices[0])));
	}
	return res / 2;
}


float3 multV(float3 p1, float3 p2) {
    float3 res = {p1.y * p2.z - p1.z * p2.y,
				 p1.z * p2.x - p1.x * p2.z,
				 p1.x * p2.y - p1.y * p2.x};
	return res;
}


inline float length(float3 p) {
	return sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

//Unused!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
float3 normal(polygon p) {
	float3 res = multV(sub(p.vertices[1], p.vertices[0]),
					sub(p.vertices[2], p.vertices[0]));
    return mult(res, 1.0 / length(res));
}


float3 center(patch p) {
    float3 c = {0, 0, 0};
    for (int i = 0; i < p.length; ++i) {
        c = sum(c, p.vertices[i]);
    }
    return mult(c, 1.0 / p.length);
}


float3 randomPointInSquare(polygon p) {
    float3 p1 = sub(p.vertices[1], p.vertices[0]);
    float3 p2 = sub(p.vertices[3], p.vertices[0]);
    float l1 = (float)rand() / RAND_MAX;
    float l2 = (float)rand() / RAND_MAX;
    return sum(p.vertices[0], sum(mult(p1, l1), mult(p2, l2)));
}

float3 polarizePointInPolygon(polygon pl, float3 pnt) {
    float3 c = center(pl);
    return sum(c, sub(c, pnt));
}

float3 randomPoint(patch p) {
    float * weightes = calloc(p.length, sizeof(*weightes));
    float s = 0;
    for (int i = 0; i < p.length; ++i) {
        weightes[i] = rand();
        s += weightes[i];
    }
    for (int i = 0; i < p.length; ++i) {
        weightes[i] /= s;
    }
    float3 c = center(p);
    float3 res = c;
    for (int i = 0; i < p.length; ++i) {
        res = sum(res, mult(sub(p.vertices[i], c), weightes[i]));
    }
    free(weightes);
    return res;
}


float multS(float3 p1, float3 p2) {
    return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
}


float cosV(float3 p1, float3 p2) {
	return multS(p1, p2) / length(p1) / length(p2);
}


int inPolygon(polygon pl, float3 p) {
	float sq = 0;
	for (int i = 1; i <= pl.length; ++i) {
        sq += length(multV(sub(pl.vertices[i % pl.length], p),
							sub(pl.vertices[i - 1], p)));
	}
	return fabs(sq / 2 - square(pl)) < DBL_EPSILON;
}


float distance(polygon pl, float3 p) {
    float d = -multS(pl.normal, pl.vertices[0]);
    return multS(pl.normal, p) + d;
}


int checkIntersection(polygon pl, float3 p1, float3 p2) {
    float3 aug = sub(p2, p1);
    float d = -multS(pl.normal, pl.vertices[0]);
    float t = multS(pl.normal, aug);
    if (fabs(t) < DBL_EPSILON) return 0;
    t =  -(d + multS(pl.normal, p1)) / t;
    if (t <= 0 || t > 1) {
        return 0;
    }
    return inPolygon(pl, sum(p1, mult(aug, t)));
}
