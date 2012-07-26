#ifndef RAYTRI_H
#define RAYTRI_H

#include <stdint.h>

typedef int32_t    fp_t;
#define FPP        16
#define FPMUL(x,y)	((fp_t)(((int64_t)(x) * (int64_t)(y)) >> FPP))
#define FPDIV(x,y)	((fp_t)(((int64_t)(x) << FPP) / (y)))
#define FTOFP(x)    ((fp_t)((x) * (1<<FPP) + ((x) >= 0 ? 0.5 : -0.5)))
#define FPTOF(x)    ((float)(x) / (1<<FPP))
#define FPONE       (fp_t)(1<<FPP)
#define FPZERO      (fp_t)0

extern int fixed_intersect_triangle(
	fp_t orig[3], fp_t dir[3],
	fp_t vert0[3], fp_t vert1[3], fp_t vert2[3],
	fp_t *t, fp_t *u, fp_t *v);

extern int intersect_triangle(
	float orig[3], float dir[3],
	float vert0[3], float vert1[3], float vert2[3],
	float *t, float *u, float *v);

extern int intersect_triangle1(
	float orig[3], float dir[3],
	float vert0[3], float vert1[3], float vert2[3],
	float *t, float *u, float *v);

extern int intersect_triangle2(
	float orig[3], float dir[3],
	float vert0[3], float vert1[3], float vert2[3],
	float *t, float *u, float *v);

extern int intersect_triangle3(
	float orig[3], float dir[3],
	float vert0[3], float vert1[3], float vert2[3],
	float *t, float *u, float *v);


#endif //RAYTRI_H
