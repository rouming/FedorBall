/***************************************************************************
 * Dodecahedron coords (verteces and faces)
 **************************************************************************/
#ifndef COORDS_H
#define COORDS_H

#include "fixed.h"

/*
 * PINOUT FOR ONE TLC
 *  ------------------
 *
 * 15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
 *  G| B| R| G| B| R| G| B| R| G| B| R| G| B| R|
 *
 *
 * BOTTOM
 * ------
 *
 *
 *    @ 8             @ 9
 *
 *        @ 3      @ 4
 *
 *
 *      @ 2         @ 5
 *  @ 7                 @ 10
 *
 *            @ 1
 *
 *            @ 6
 *
 *
 * ^    ^    ^    ^    ^    ^
 * |    |    |    |    |    |
 *
 *            VIEWER
 *
 * TOP
 * ---
 *
 *            @ 6
 *
 *            @ 1
 *  @ 10                @ 7
 *      @ 5         @ 2
 *
 *
 *       @ 4     @ 3
 *
 *    @ 9            @ 8
 *
 *
 * ^    ^    ^    ^    ^    ^
 * |    |    |    |    |    |
 *
 *            VIEWER
 *
 */

/* TLC5940 pinout index */
#define TLC0 16*0
#define TLC1 16*1
#define TLC2 16*2
#define TLC3 16*3

/* Dodecahedron 20 real coords verteces: every row is a vertex (x,y,z).
 * Original coords were taken from
 * http://www.maths.surrey.ac.uk/hosted-sites/R.Knott/Fibonacci/phi3DGeom.html
 * Actually, on the site phi_1 means phi-1, not 1-phi,
 * but this my typo does not matter at all.
 *
 * Then verteces were normalized and rotated on 31.717 degrees around X axis
 * (OpenGL coordinate system)
 * (www.trinitas.ru/rus/doc/0232/006a/02321014.pdf) to have bottom face
 * lay in X plane
 *
 * (see output of ../gl_dodecahedron_src/dodecahedron and
 *  sources: ../gl_dodecahedron_src/main.cpp:drawDodecahedron for details)
 */
const static fp_t s_coords_vert[] =
{
	FTOFP(0.00000f),		FTOFP(-0.79465f),		FTOFP(0.60707f),
	FTOFP(0.00000f),		FTOFP(0.79465f),		FTOFP(-0.60707f),
	FTOFP(0.93417f),		FTOFP(0.18759f),		FTOFP(-0.30353f),
	FTOFP(0.93417f),		FTOFP(-0.18759f),		FTOFP(0.30353f),
	FTOFP(-0.93417f),		FTOFP(0.18759f),		FTOFP(-0.30353f),
	FTOFP(-0.93417f),		FTOFP(-0.18759f),		FTOFP(0.30353f),
	FTOFP(-0.35682f),		FTOFP(0.79466f),		FTOFP(0.49112f),
	FTOFP(-0.35682f),		FTOFP(-0.79466f),		FTOFP(-0.49112f),
	FTOFP(0.35682f),		FTOFP(0.79466f),		FTOFP(0.49112f),
	FTOFP(0.35682f),		FTOFP(-0.79466f),		FTOFP(-0.49112f),
	FTOFP(0.57735f),		FTOFP(0.18760f),		FTOFP(0.79465f),
	FTOFP(0.57735f),		FTOFP(0.79465f),		FTOFP(-0.18760f),
	FTOFP(0.57735f),		FTOFP(-0.79465f),		FTOFP(0.18760f),
	FTOFP(0.57735f),		FTOFP(-0.18760f),		FTOFP(-0.79465f),
	FTOFP(-0.57735f),		FTOFP(0.18760f),		FTOFP(0.79465f),
	FTOFP(-0.57735f),		FTOFP(0.79465f),		FTOFP(-0.18760f),
	FTOFP(-0.57735f),		FTOFP(-0.79465f),		FTOFP(0.18760f),
	FTOFP(-0.57735f),		FTOFP(-0.18760f),		FTOFP(-0.79465f),
};

/* Dodecahedron 20 leds verteces: every row is a led index (r,g,b)  */
const static uint8_t s_leds_vert[] =
{
	/*  #0 TLC5940 */
	TLC0+  1,  TLC0+  2,  TLC0+  3,
	TLC0+  4,  TLC0+  5,  TLC0+  6,
	TLC0+  7,  TLC0+  8,  TLC0+  9,
	TLC0+ 10,  TLC0+ 11,  TLC0+ 12,
	TLC0+ 13,  TLC0+  4,  TLC0+ 15,

	/*  #1 TLC5940 */
	TLC1+  1,  TLC1+  2,  TLC1+  3,
	TLC1+  4,  TLC1+  5,  TLC1+  6,
	TLC1+  7,  TLC1+  8,  TLC1+  9,
	TLC1+ 10,  TLC1+ 11,  TLC1+ 12,
	TLC1+ 13,  TLC1+  4,  TLC1+ 15,

	/*  #2 TLC5940 */
	TLC2+  1,  TLC2+  2,  TLC2+  3,
	TLC2+  4,  TLC2+  5,  TLC2+  6,
	TLC2+  7,  TLC2+  8,  TLC2+  9,
	TLC2+ 10,  TLC2+ 11,  TLC2+ 12,
	TLC2+ 13,  TLC2+  4,  TLC2+ 15,

	/*  #3 TLC5940 */
	TLC3+  1,  TLC3+  2,  TLC3+  3,
	TLC3+  4,  TLC3+  5,  TLC3+  6,
	TLC3+  7,  TLC3+  8,  TLC3+  9,
	TLC3+ 10,  TLC3+ 11,  TLC3+ 12,
	TLC3+ 13,  TLC3+  4,  TLC3+ 15
};

/* Dodecahedron triangle faces from verteces,
   i.e. 3 triangles in one row is a pentagon.
   This array will be used for ray triangle
   intersection calculation */
const static uint8_t s_vert_tri_faces[] =
{
	12, 2,  0,      12, 0,  14,     12,  14,  5,
	10, 8,  16,     10, 16, 2,      10,  2,   12,
	17, 6,  7,      17, 7,  16,     17,  16,  8,
	16, 7,  18,     16, 18, 0,      16,  0,   2,
	0,  18, 9,      0,  9,  11,     0,   11,  14,
	5,  14, 11,     5,  11, 15,     5,   15,  4,
	13, 4,  15,     13, 15, 1,      13,  1,   3,
	17, 3,  1,      17, 1,  19,     17,  19,  6,
	10, 12, 5,      10, 5,  4,      10,  4,   13,
	3,  17, 8,      3,  8,  10,     3,   10,  13,
	11, 9,  19,     11, 19, 1,      11,  1,   15,
	9,  18, 7,      9,  7,  6,      9,   6,   19
};

/* Dodecahedron 12 pentagons from verteces,
   i.e. one row in array is a pentagon */
const static uint8_t s_vert_penta_faces[] =
{
	12,  2,  0, 14,  5,
	10,  8, 16,  2, 12,
	17,  6,  7, 16,  8,
	16,  7, 18,  0,  2,
	0, 18,  9, 11, 14,
	5, 14, 11, 15,  4,
	13,  4, 15,  1,  3,
	17,  3,  1, 19,  6,
	10, 12,  5,  4, 13,
	3, 17,  8, 10, 13,
	11,  9, 19,  1, 15,
	9, 18,  7,  6, 19
};

#endif //COORDS_H
