/***************************************************************************
 * Dodecahedron coords (verteces and faces)
 **************************************************************************/
#ifndef COORDS_H
#define COORDS_H

#include "fixed.h"

/*
 * BOTTOM (clockwise direction)
 * ------
 *
 *
 *    @8             @9
 *
 *        @3      @4
 *
 *
 *      @2         @5
 *  @7                 @10
 *
 *            @1
 *
 *            @6
 *
 *
 * ^    ^    ^    ^    ^    ^
 * |    |    |    |    |    |
 *
 *            VIEWER
 *
 * leds verteces:
 *    TLC0: @1->0, @2->18, @3->9,  @4->11, @5->14
 *    TLC1: @6->2, @7->7,  @8->19, @9->15, @10->5
 *
 *
 * TOP (clockwise direction)
 * ---
 *
 *            @6
 *
 *            @2
 *  @10                @7
 *      @1         @3
 *
 *
 *       @5     @4
 *
 *    @9            @8
 *
 *
 * ^    ^    ^    ^    ^    ^
 * |    |    |    |    |    |
 *
 *            VIEWER
 *
 * leds verteces:
 *    TLC2: @1->17, @2->3,  @3->13,  @4->10,  @5 ->8
 *    TLC3: @6->1,  @7->4,  @8->12,  @9->16,  @10->6
 */

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
	0,		-203,		155,
	0,		48,		-251,
	0,		-48,		251,
	0,		203,		-155,
	239,		48,		-78,
	239,		-48,		78,
	-239,		48,		-78,
	-239,		-48,		78,
	-91,		203,		126,
	-91,		-203,		-126,
	91,		203,		126,
	91,		-203,		-126,
	148,		48,		203,
	148,		203,		-48,
	148,		-203,		48,
	148,		-48,		-203,
	-148,		48,		203,
	-148,		203,		-48,
	-148,		-203,		48,
	-148,		-48,		-203,
};

/*
 * PINOUT FOR ONE TLC
 *  ------------------
 *
 * 15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
 *  G| B| R| G| B| R| G| B| R| G| B| R| G| B| R|
 */
#define R_TLC(tlc, led) (tlc*16 + led*3 + 1)
#define G_TLC(tlc, led) (tlc*16 + led*3 + 3)
#define B_TLC(tlc, led) (tlc*16 + led*3 + 2)

/* Dodecahedron 20 leds verteces: every row is a tlc led (r,g,b)  */
const static uint8_t s_leds_vert[] =
{
	R_TLC(0, 0),	G_TLC(0, 0),	B_TLC(0, 0),
	R_TLC(3, 0),	G_TLC(3, 0),	B_TLC(3, 0),
	R_TLC(1, 0),	G_TLC(1, 0),	B_TLC(1, 0),
	R_TLC(2, 1),	G_TLC(2, 1),	B_TLC(2, 1),
	R_TLC(3, 1),	G_TLC(3, 1),	B_TLC(3, 1),
	R_TLC(1, 4),	G_TLC(1, 4),	B_TLC(1, 4),
	R_TLC(3, 4),	G_TLC(3, 4),	B_TLC(3, 4),
	R_TLC(1, 1),	G_TLC(1, 1),	B_TLC(1, 1),
	R_TLC(2, 4),	G_TLC(2, 4),	B_TLC(2, 4),
	R_TLC(0, 2),	G_TLC(0, 2),	B_TLC(0, 2),
	R_TLC(2, 3),	G_TLC(2, 3),	B_TLC(2, 3),
	R_TLC(0, 3),	G_TLC(0, 3),	B_TLC(0, 3),
	R_TLC(3, 2),	G_TLC(3, 2),	B_TLC(3, 2),
	R_TLC(2, 2),	G_TLC(2, 2),	B_TLC(2, 2),
	R_TLC(0, 4),	G_TLC(0, 4),	B_TLC(0, 4),
	R_TLC(1, 3),	G_TLC(1, 3),	B_TLC(1, 3),
	R_TLC(3, 3),	G_TLC(3, 3),	B_TLC(3, 3),
	R_TLC(2, 0),	G_TLC(2, 0),	B_TLC(2, 0),
	R_TLC(0, 1),	G_TLC(0, 1),	B_TLC(0, 1),
	R_TLC(1, 2),	G_TLC(1, 2),	B_TLC(1, 2),
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

#endif //COORDS_H
