/* Ray-Triangle Intersection Test Routines          */
/* Different optimizations of my and Ben Trumbore's */
/* code from journals of graphics tools (JGT)       */
/* http://www.acm.org/jgt/                          */
/* by Tomas Moller, May 2000                        */

#ifndef RAYTRI_H
#define RAYTRI_H

#include "fixed.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FIXED_INTERSECT

int fixed_intersect_triangle(
	fp_t orig[3], fp_t dir[3],
	fp_t vert0[3], fp_t vert1[3], fp_t vert2[3],
	fp_t *t, fp_t *u, fp_t *v);

#else

int intersect_triangle(
	float orig[3], float dir[3],
	float vert0[3], float vert1[3], float vert2[3],
	float *t, float *u, float *v);

#endif //FIXED_INTERSECT

#ifdef INTERSECT_BENCHMARK

int intersect_triangle1(
	float orig[3], float dir[3],
	float vert0[3], float vert1[3], float vert2[3],
	float *t, float *u, float *v);

int intersect_triangle2(
	float orig[3], float dir[3],
	float vert0[3], float vert1[3], float vert2[3],
	float *t, float *u, float *v);

int intersect_triangle3(
	float orig[3], float dir[3],
	float vert0[3], float vert1[3], float vert2[3],
	float *t, float *u, float *v);

#endif //INTERSECT_BENCHMARK

#ifdef __cplusplus
}
#endif

#endif //RAYTRI_H
