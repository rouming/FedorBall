#ifndef RAYTRI_H
#define RAYTRI_H

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
