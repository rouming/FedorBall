///////////////////////////////////////////////////////////////////////////////
// main.cpp
// ========
// testing vertex array (glDrawElements, glDrawArrays)
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2005-10-04
// UPDATED: 2012-04-11
///////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/glext.h>
#endif

//#include "glext.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <math.h>

#include <stdio.h>

using std::stringstream;
using std::cout;
using std::endl;
using std::ends;


// GLUT CALLBACK functions
void displayCB();
void reshapeCB(int w, int h);
void timerCB(int millisec);
void keyboardCB(unsigned char key, int x, int y);
void mouseCB(int button, int stat, int x, int y);
void mouseMotionCB(int x, int y);

void initGL();
int  initGLUT(int argc, char **argv);
bool initSharedMem();
void clearSharedMem();
void initLights();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void drawString(const char *str, int x, int y, float color[4], void *font);
void drawString3D(const char *str, float pos[3], float color[4], void *font);
void toOrtho();
void toPerspective();
void draw1();
void draw2();
void draw3();
void draw4();
void draw5();


// constants
const int   SCREEN_WIDTH    = 400;
const int   SCREEN_HEIGHT   = 300;
const float CAMERA_DISTANCE = 10.0f;
const int   TEXT_WIDTH      = 8;
const int   TEXT_HEIGHT     = 13;


// global variables
static void *font = GLUT_BITMAP_8_BY_13;
static int screenWidth;
static int screenHeight;
static bool mouseLeftDown;
static bool mouseRightDown;
static bool mouseMiddleDown;
static float mouseX, mouseY;
static float cameraAngleX;
static float cameraAngleY;
static float cameraDistance;
static int drawMode = 0;
static int maxVertices;
static int maxIndices;

#ifdef _WIN32
// function pointer to OpenGL extensions
// glDrawRangeElements() defined in OpenGL v1.2 or greater
PFNGLDRAWRANGEELEMENTSPROC pglDrawRangeElements = 0;
#define glDrawRangeElements pglDrawRangeElements
#endif



// cube ///////////////////////////////////////////////////////////////////////
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3

// vertex coords array for glDrawArrays() =====================================
// A cube has 6 sides and each side has 2 triangles, therefore, a cube consists
// of 36 vertices (6 sides * 2 tris * 3 vertices = 36 vertices). And, each
// vertex is 3 components (x,y,z) of floats, therefore, the size of vertex
// array is 108 floats (36 * 3 = 108).
GLfloat vertices1[] = { 1, 1, 1,  -1, 1, 1,  -1,-1, 1,      // v0-v1-v2 (front)
                       -1,-1, 1,   1,-1, 1,   1, 1, 1,      // v2-v3-v0

                        1, 1, 1,   1,-1, 1,   1,-1,-1,      // v0-v3-v4 (right)
                        1,-1,-1,   1, 1,-1,   1, 1, 1,      // v4-v5-v0

                        1, 1, 1,   1, 1,-1,  -1, 1,-1,      // v0-v5-v6 (top)
                       -1, 1,-1,  -1, 1, 1,   1, 1, 1,      // v6-v1-v0

                       -1, 1, 1,  -1, 1,-1,  -1,-1,-1,      // v1-v6-v7 (left)
                       -1,-1,-1,  -1,-1, 1,  -1, 1, 1,      // v7-v2-v1

                       -1,-1,-1,   1,-1,-1,   1,-1, 1,      // v7-v4-v3 (bottom)
                        1,-1, 1,  -1,-1, 1,  -1,-1,-1,      // v3-v2-v7

                        1,-1,-1,  -1,-1,-1,  -1, 1,-1,      // v4-v7-v6 (back)
                       -1, 1,-1,   1, 1,-1,   1,-1,-1 };    // v6-v5-v4

// normal array
GLfloat normals1[]  = { 0, 0, 1,   0, 0, 1,   0, 0, 1,      // v0-v1-v2 (front)
                        0, 0, 1,   0, 0, 1,   0, 0, 1,      // v2-v3-v0

                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v0-v3-v4 (right)
                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v4-v5-v0

                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v0-v5-v6 (top)
                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v6-v1-v0

                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v1-v6-v7 (left)
                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v7-v2-v1

                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v7-v4-v3 (bottom)
                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v3-v2-v7

                        0, 0,-1,   0, 0,-1,   0, 0,-1,      // v4-v7-v6 (back)
                        0, 0,-1,   0, 0,-1,   0, 0,-1 };    // v6-v5-v4

// color array
GLfloat colors1[]   = { 1, 1, 1,   1, 1, 0,   1, 0, 0,      // v0-v1-v2 (front)
                        1, 0, 0,   1, 0, 1,   1, 1, 1,      // v2-v3-v0

                        1, 1, 1,   1, 0, 1,   0, 0, 1,      // v0-v3-v4 (right)
                        0, 0, 1,   0, 1, 1,   1, 1, 1,      // v4-v5-v0

                        1, 1, 1,   0, 1, 1,   0, 1, 0,      // v0-v5-v6 (top)
                        0, 1, 0,   1, 1, 0,   1, 1, 1,      // v6-v1-v0

                        1, 1, 0,   0, 1, 0,   0, 0, 0,      // v1-v6-v7 (left)
                        0, 0, 0,   1, 0, 0,   1, 1, 0,      // v7-v2-v1

                        0, 0, 0,   0, 0, 1,   1, 0, 1,      // v7-v4-v3 (bottom)
                        1, 0, 1,   1, 0, 0,   0, 0, 0,      // v3-v2-v7

                        0, 0, 1,   0, 0, 0,   0, 1, 0,      // v4-v7-v6 (back)
                        0, 1, 0,   0, 1, 1,   0, 0, 1 };    // v6-v5-v4



// vertex array for glDrawElements() and glDrawRangeElement() =================
// Notice that the vertex array for glDrawArray() 
GLfloat vertices2[] = { 1, 1, 1,  -1, 1, 1,  -1,-1, 1,   1,-1, 1,   // v0,v1,v2,v3 (front)
                        1, 1, 1,   1,-1, 1,   1,-1,-1,   1, 1,-1,   // v0,v3,v4,v5 (right)
                        1, 1, 1,   1, 1,-1,  -1, 1,-1,  -1, 1, 1,   // v0,v5,v6,v1 (top)
                       -1, 1, 1,  -1, 1,-1,  -1,-1,-1,  -1,-1, 1,   // v1,v6,v7,v2 (left)
                       -1,-1,-1,   1,-1,-1,   1,-1, 1,  -1,-1, 1,   // v7,v4,v3,v2 (bottom)
                        1,-1,-1,  -1,-1,-1,  -1, 1,-1,   1, 1,-1 }; // v4,v7,v6,v5 (back)

// normal array
GLfloat normals2[]  = { 0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 1,   // v0,v1,v2,v3 (front)
                        1, 0, 0,   1, 0, 0,   1, 0, 0,   1, 0, 0,   // v0,v3,v4,v5 (right)
                        0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,   // v0,v5,v6,v1 (top)
                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,  -1, 0, 0,   // v1,v6,v7,v2 (left)
                        0,-1, 0,   0,-1, 0,   0,-1, 0,   0,-1, 0,   // v7,v4,v3,v2 (bottom)
                        0, 0,-1,   0, 0,-1,   0, 0,-1,   0, 0,-1 }; // v4,v7,v6,v5 (back)

// color array
GLfloat colors2[]   = { 1, 1, 1,   1, 1, 1,   1, 1, 1,   1, 1, 1,   // v0,v1,v2,v3 (front)
                        0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 1,   // v0,v3,v4,v5 (right)
                        1, 0, 1,   1, 0, 1,   1, 0, 1,   1, 0, 1,   // v0,v5,v6,v1 (top)
                        1, 1, 0,   1, 1, 0,   1, 1, 0,   1, 1, 0,   // v1,v6,v7,v2 (left)
                        4, 0, 1,   4, 0, 1,   4, 0, 1,   4, 0, 1,   // v7,v4,v3,v2 (bottom)
                        0, 1, 1,   0, 1, 1,   0, 1, 1,   0, 1, 1 }; // v4,v7,v6,v5 (back)

// index array of vertex array for glDrawElements() & glDrawRangeElement()
// Notice the indices are listed straight from beginning to end as exactly
// same order of vertex array without hopping, because of different normals at
// a shared vertex. For this case, glDrawArrays() and glDrawElements() have no
// difference.
GLubyte indices[]  = { 0, 1, 2,   2, 3, 0,      // front
                       4, 5, 6,   6, 7, 4,      // right
                       8, 9,10,  10,11, 8,      // top
                      12,13,14,  14,15,12,      // left
                      16,17,18,  18,19,16,      // bottom
                      20,21,22,  22,23,20 };    // back



// interleaved vertex array for glDrawElements() & glDrawRangeElements() ======
// All vertex attributes (position, normalm, color) are packed together as a
// struct or set, for example, ((VNC)(VNC)(VNC)...).
// It is called an array of struct, and provides better memory locality.
GLfloat vertices3[] = { 1, 1, 1,   0, 0, 1,   1, 1, 1,              // v0 (front)
                       -1, 1, 1,   0, 0, 1,   1, 1, 0,              // v1
                       -1,-1, 1,   0, 0, 1,   1, 0, 0,              // v2
                        1,-1, 1,   0, 0, 1,   1, 0, 1,              // v3

                        1, 1, 1,   1, 0, 0,   1, 1, 1,              // v0 (right)
                        1,-1, 1,   1, 0, 0,   1, 0, 1,              // v3
                        1,-1,-1,   1, 0, 0,   0, 0, 1,              // v4
                        1, 1,-1,   1, 0, 0,   0, 1, 1,              // v5

                        1, 1, 1,   0, 1, 0,   1, 1, 1,              // v0 (top)
                        1, 1,-1,   0, 1, 0,   0, 1, 1,              // v5
                       -1, 1,-1,   0, 1, 0,   0, 1, 0,              // v6
                       -1, 1, 1,   0, 1, 0,   1, 1, 0,              // v1

                       -1, 1, 1,  -1, 0, 0,   1, 1, 0,              // v1 (left)
                       -1, 1,-1,  -1, 0, 0,   0, 1, 0,              // v6
                       -1,-1,-1,  -1, 0, 0,   0, 0, 0,              // v7
                       -1,-1, 1,  -1, 0, 0,   1, 0, 0,              // v2

                       -1,-1,-1,   0,-1, 0,   0, 0, 0,              // v7 (bottom)
                        1,-1,-1,   0,-1, 0,   0, 0, 1,              // v4
                        1,-1, 1,   0,-1, 0,   1, 0, 1,              // v3
                       -1,-1, 1,   0,-1, 0,   1, 0, 0,              // v2

                        1,-1,-1,   0, 0,-1,   0, 0, 1,              // v4 (back)
                       -1,-1,-1,   0, 0,-1,   0, 0, 0,              // v7
                       -1, 1,-1,   0, 0,-1,   0, 1, 0,              // v6
                        1, 1,-1,   0, 0,-1,   0, 1, 1 };            // v5


struct point
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

bool operator<(const point& p1, const point& p2)
{
	const GLfloat epsilon = 1e-8;

	if (p1.x < p2.x - epsilon) return true;
	if (p1.x > p2.x + epsilon) return false;

	if (p1.y < p2.y - epsilon) return true;
	if (p1.y > p2.y + epsilon) return false;

	if (p1.z < p2.z - epsilon) return true;

  return false;

	/*
	GLfloat sz1 = sqrt(p1.x*p1.x + p1.y*p1.y + p1.z*p1.z);
	GLfloat sz2 = sqrt(p2.x*p2.x + p2.y*p2.y + p2.z*p2.z);
	return sz1 < sz2;
	*/
	return p1.x < p2.x || p1.y < p2.y || p1.z < p2.z;
}

bool operator==(const point& p1, const point& p2)
{
	return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
}

struct vector
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

struct triangle
{
	point p0;
	point p1;
	point p2;
};

void create_triangle(GLfloat *vertices, unsigned int idx,
							triangle& tr)
{
	tr.p0.x = vertices[idx + 0];
	tr.p0.y = vertices[idx + 1];
	tr.p0.z = vertices[idx + 2];

	tr.p1.x = vertices[idx + 3];
	tr.p1.y = vertices[idx + 4];
	tr.p1.z = vertices[idx + 5];

	tr.p2.x = vertices[idx + 6];
	tr.p2.y = vertices[idx + 7];
	tr.p2.z = vertices[idx + 8];
};

void create_triangle(GLfloat *vertices, GLbyte *faces,
					 unsigned int idx, triangle& tr)
{
	tr.p0.x = vertices[faces[idx + 0] * 3 + 0];
	tr.p0.y = vertices[faces[idx + 0] * 3 + 1];
	tr.p0.z = vertices[faces[idx + 0] * 3 + 2];

	tr.p1.x = vertices[faces[idx + 1] * 3 + 0];
	tr.p1.y = vertices[faces[idx + 1] * 3 + 1];
	tr.p1.z = vertices[faces[idx + 1] * 3 + 2];

	tr.p2.x = vertices[faces[idx + 2] * 3 + 0];
	tr.p2.y = vertices[faces[idx + 2] * 3 + 1];
	tr.p2.z = vertices[faces[idx + 2] * 3 + 2];
};


static void vector_cross(const vector& a, const vector& b, vector& r)
{
	r.x = a.y * b.z - a.z * b.y;
	r.y = a.z * b.x - a.x * b.z;
	r.z = a.x * b.y - a.y * b.x;
}

static void vector_normalize(vector& v)
{
#define SMALL_VECTOR (1e-4)
	GLfloat size = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	if (size < SMALL_VECTOR) {
		printf("Vector is too small!\n");
		exit(-1);
	}

	v.x /= size;
	v.y /= size;
	v.z /= size;
}

static void normal(const triangle& tr, vector& n)
{
	vector v1 = {tr.p1.x - tr.p0.x, tr.p1.y - tr.p0.y, tr.p1.z - tr.p0.z};
	vector v2 = {tr.p2.x - tr.p0.x, tr.p2.y - tr.p0.y, tr.p2.z - tr.p0.z};
	vector_cross(v1, v2, n);
	vector_normalize(n);
}

static float phi = (1.0 + sqrt(5.0)) / 2.0;
static float phi_1 = 1-phi;
static float phi2 = phi * phi;
static float phi3 = phi * phi * phi;
static GLfloat dod_orig_vert[] =
{
/*
	0,   phi,  phi3, // V4   0
	0,  -phi,  phi3, // V8   1

	phi2,  phi2,  phi2, // V11  2
	-phi2,  phi2,  phi2, // V13  3
	-phi2, -phi2,  phi2, // V16  4
	phi2, -phi2,  phi2, // V18  5

	phi3,     0,   phi, // V20  6
	-phi3,     0,   phi, // V23  7
	phi,  phi3,     0, // V28  8
	-phi,  phi3,     0, // V30  9
	-phi, -phi3,     0, // V34 10
	phi, -phi3,     0, // V36 11
	phi3,     0,  -phi, // V38 12
	-phi3,     0,  -phi, // V41 13

	phi2,  phi2, -phi2, // V45 14
	-phi2,  phi2, -phi2, // V47 15
	-phi2, -phi2, -phi2, // V50 16
	phi2, -phi2, -phi2, // V52 17

	0,   phi, -phi3, // V56 18
	0,  -phi, -phi3  // V60 19
*/

	0, phi_1, phi,
	0, phi_1, -phi,
	0, -phi_1, phi,
	0, -phi_1, -phi,
	phi, 0, phi_1,
	phi, 0, -phi_1,
	-phi, 0, phi_1,
	-phi, 0, -phi_1,
	phi_1, phi, 0,
	phi_1, -phi, 0,
	-phi_1, phi, 0,
	-phi_1, -phi, 0,
	1, 1, 1,
	1, 1, -1,
	1, -1, 1,
	1, -1, -1,
	-1, 1, 1,
	-1, 1, -1,
	-1, -1, 1,
	-1, -1, -1

};

static GLbyte dod_orig_faces[] =
{
/*
	1, 0, 4,   0, 3, 4,   3, 7, 4,
	0, 1, 2,   1, 5, 2,   5, 6, 2,

	0, 2, 3,   2, 8, 3,   8, 9, 3,
	1, 4, 5,   4,10, 5,   10,11, 5,

	3, 9, 7,   9,15, 7,   15,13, 7,
	4, 7,10,   7,13,10,   13,16,10,
	5,11, 6,   11,17, 6,  17,12, 6,
	2, 6, 8,   6,12, 8,   12,14, 8,

	9, 8,15,   8,14,15,   14,18,15,
	11,10,17,  10,16,17,  16,19,17,
	13,15,16,  15,18,16,  18,19,16,
	12,17,14,  17,19,14,  19,18,14
*/

	12, 2, 0,   12, 0, 14,  12, 14, 5,
	10, 8, 16,  10, 16, 2,  10, 2, 12,
	17, 6, 7,   17, 7, 16,  17, 16, 8,
	16, 7, 18,  16, 18, 0,  16, 0, 2,
	0,  18, 9,  0,  9, 11,  0, 11, 14,
	5, 14, 11,  5, 11, 15,  5, 15, 4,
	13, 4, 15,  13, 15, 1,  13, 1, 3,
	17, 3, 1,   17, 1, 19,  17, 19, 6,

	10, 12, 5,  10, 5, 4,   10, 4, 13,
	3, 17, 8,   3, 8 ,10,   3, 10, 13,
	11, 9, 19,  11, 19, 1,  11, 1, 15,
	9, 18, 7,   9, 7, 6,    9, 6, 19, //??

};

GLfloat dod_colors[] =
{
	1, 1, 1,   1, 1, 1,   1, 1, 1,
	1, 0, 1,   1, 0, 1,   1, 0, 1,

	0, 0, 1,   0, 0, 1,   0, 0, 1,
	0, 1, 1,   0, 1, 1,   0, 1, 1,

	1, 1, 0,   1, 1, 0,   1, 1, 0,
	1, 0, 0,   1, 0, 0,   1, 0, 0,

	5, 0, 1,   5, 0, 1,   5, 0, 1,
	0, 5, 0,   0, 5, 0,   0, 5, 0,

	3, 3, 1,   3, 3, 1,   3, 3, 1,
	4, 0, 3,   4, 0, 3,    4, 0, 3,

	4, 0, 1,   4, 0, 1,   4, 0, 1,
	1, 1, 5,   1, 1, 5,   1, 1, 5
};

//faces * verteces_per_face * cords_in_3d
static GLfloat dod_vert[12*5*3];
//faces * verteces_per_face * cords_in_3d
static GLfloat dod_normals[12*5*3];
//faces * triangles_per_face * points_per_triangle
static GLbyte dod_faces[12*3*3];

///////////////////////////////////////////////////////////////////////////////
// draw 1: immediate mode
// 78 calls = 36 glVertex*() calls + 36 glColor*() calls + 6 glNormal*() calls
///////////////////////////////////////////////////////////////////////////////
void draw1()
{
    glPushMatrix();
    glTranslatef(-2, 2, 0); // move to upper left corner

    glBegin(GL_TRIANGLES);
        // front faces
        glNormal3f(0,0,1);
        // face v0-v1-v2
        glColor3f(1,1,1);
        glVertex3f(1,1,1);
        glColor3f(1,1,0);
        glVertex3f(-1,1,1);
        glColor3f(1,0,0);
        glVertex3f(-1,-1,1);
        // face v2-v3-v0
        glColor3f(1,0,0);
        glVertex3f(-1,-1,1);
        glColor3f(1,0,1);
        glVertex3f(1,-1,1);
        glColor3f(1,1,1);
        glVertex3f(1,1,1);

        // right faces
        glNormal3f(1,0,0);
        // face v0-v3-v4
        glColor3f(1,1,1);
        glVertex3f(1,1,1);
        glColor3f(1,0,1);
        glVertex3f(1,-1,1);
        glColor3f(0,0,1);
        glVertex3f(1,-1,-1);
        // face v4-v5-v0
        glColor3f(0,0,1);
        glVertex3f(1,-1,-1);
        glColor3f(0,1,1);
        glVertex3f(1,1,-1);
        glColor3f(1,1,1);
        glVertex3f(1,1,1);

        // top faces
        glNormal3f(0,1,0);
        // face v0-v5-v6
        glColor3f(1,1,1);
        glVertex3f(1,1,1);
        glColor3f(0,1,1);
        glVertex3f(1,1,-1);
        glColor3f(0,1,0);
        glVertex3f(-1,1,-1);
        // face v6-v1-v0
        glColor3f(0,1,0);
        glVertex3f(-1,1,-1);
        glColor3f(1,1,0);
        glVertex3f(-1,1,1);
        glColor3f(1,1,1);
        glVertex3f(1,1,1);

        // left faces
        glNormal3f(-1,0,0);
        // face  v1-v6-v7
        glColor3f(1,1,0);
        glVertex3f(-1,1,1);
        glColor3f(0,1,0);
        glVertex3f(-1,1,-1);
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);
        // face v7-v2-v1
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);
        glColor3f(1,0,0);
        glVertex3f(-1,-1,1);
        glColor3f(1,1,0);
        glVertex3f(-1,1,1);

        // bottom faces
        glNormal3f(0,-1,0);
        // face v7-v4-v3
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);
        glColor3f(0,0,1);
        glVertex3f(1,-1,-1);
        glColor3f(1,0,1);
        glVertex3f(1,-1,1);
        // face v3-v2-v7
        glColor3f(1,0,1);
        glVertex3f(1,-1,1);
        glColor3f(1,0,0);
        glVertex3f(-1,-1,1);
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);

        // back faces
        glNormal3f(0,0,-1);
        // face v4-v7-v6
        glColor3f(0,0,1);
        glVertex3f(1,-1,-1);
        glColor3f(0,0,0);
        glVertex3f(-1,-1,-1);
        glColor3f(0,1,0);
        glVertex3f(-1,1,-1);
        // face v6-v5-v4
        glColor3f(0,1,0);
        glVertex3f(-1,1,-1);
        glColor3f(0,1,1);
        glVertex3f(1,1,-1);
        glColor3f(0,0,1);
        glVertex3f(1,-1,-1);
    glEnd();


	glBegin(GL_LINES);
	  glVertex3f(0, 0, 0); // origin of the line
	  glVertex3f(2, 1, 5); // ending point of the line
      glColor3f(0,1,1);

	glEnd( );


    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
// draw cube at upper-right corner with glDrawArrays
// A cube has only 8 vertices, but each vertex is shared for 3 different faces,
// which have different normals. Therefore, we need more than 8 vertex data to
// draw a cube. Since each face has 2 triangles, we need 6 vertices per face.
// (2 * 3 = 6) And, a cube has 6 faces, so, the total number of vertices for
// drawing a cube is 36 (= 6 faces * 6 vertices).
// Note that there are some duplicated vertex data for glDrawArray() because
// the vertex data in the vertex array must be sequentially placed in memory.
// For a cube, there are 24 unique vertex data and 12 redundant vertex data in
// the vertex array.
///////////////////////////////////////////////////////////////////////////////
void draw2()
{
	/*
    // enble and specify pointers to vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, normals1);
    glColorPointer(3, GL_FLOAT, 0, colors1);
    glVertexPointer(3, GL_FLOAT, 0, vertices1);

    glPushMatrix();
    glTranslatef(2, 2, 0);                  // move to upper-right corner

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
	*/

    // enable and specify pointers to vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    //glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, normals2);
    //glColorPointer(3, GL_FLOAT, 0, colors2);
    glVertexPointer(3, GL_FLOAT, 0, vertices2);


    glPushMatrix();
    glTranslatef(2, 2, 0);                // move to upper-right corner

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    //glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

}

///////////////////////////////////////////////////////////////////////////////
// draw cube at bottom-left corner with glDrawElements
// The main advantage of glDrawElements() over glDrawArray() is that
// glDrawElements() allows hopping around the vertex array with the associated
// index values.
// In a cube, the number of vertex data in the vertex array can be reduced to
// 24 vertices for glDrawElements().
// Note that you need an additional array (index array) to store how to traverse
// the vertext data. For a cube, we need 36 entries in the index array.
///////////////////////////////////////////////////////////////////////////////
#include <set>
#include <list>
#include <assert.h>

void draw3()
{

	//XXX
	static int s_inited = 0;
	if (!s_inited)
	{
		s_inited = 1;

		// init dod verteces and faces
		int sz = sizeof(dod_orig_faces)/sizeof(dod_orig_faces[0]);
		for (int i = 0; i < sz; i += 9) {
			int face_i = i / 9;

			std::set<point> verts;
			typedef std::list<point> faces_list;
			faces_list faces;

			for (int j = 0; j < 9; ++j) {
				GLfloat x = dod_orig_vert[dod_orig_faces[i+j] * 3 + 0];
				GLfloat y = dod_orig_vert[dod_orig_faces[i+j] * 3 + 1];
				GLfloat z = dod_orig_vert[dod_orig_faces[i+j] * 3 + 2];
				point p = {x, y, z};
				verts.insert(p);
				faces.push_back(p);
			}

			assert(verts.size() == 5);
			assert(faces.size() == 9);

			int ind = 0;
			for (faces_list::iterator it = faces.begin();
				 it != faces.end();
				 ++it, ++ind) {
				point& p = *it;
				std::set<point>::iterator v_it = verts.find(p);
				assert(v_it != verts.end());
				int id = std::distance(verts.begin(), v_it);
				assert(id >= 0 && id < 5);

				dod_vert[face_i*5*3 + id*3 + 0] = p.x;
				dod_vert[face_i*5*3 + id*3 + 1] = p.y;
				dod_vert[face_i*5*3 + id*3 + 2] = p.z;

				dod_faces[face_i*3*3 + ind] = face_i*5 + id;
			}
		}

		sz = sizeof(dod_faces)/sizeof(dod_faces[0]);
		for (int i = 0; i < sz; i += 3) {
			triangle tr;
			vector n;
			create_triangle(dod_vert, dod_faces, i, tr);
			normal(tr, n);

			dod_normals[dod_faces[i+0]*3 + 0] = n.x;
			dod_normals[dod_faces[i+0]*3 + 1] = n.y;
			dod_normals[dod_faces[i+0]*3 + 2] = n.z;

			dod_normals[dod_faces[i+1]*3 + 0] = n.x;
			dod_normals[dod_faces[i+1]*3 + 1] = n.y;
			dod_normals[dod_faces[i+1]*3 + 2] = n.z;

			dod_normals[dod_faces[i+2]*3 + 0] = n.x;
			dod_normals[dod_faces[i+2]*3 + 1] = n.y;
			dod_normals[dod_faces[i+2]*3 + 2] = n.z;
		}
	}


    // enable and specify pointers to vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    //glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    //glNormalPointer(GL_FLOAT, 0, normals2);
	glNormalPointer(GL_FLOAT, 0, dod_normals);
    //glColorPointer(3, GL_FLOAT, 0, colors2);
    //glColorPointer(3, GL_FLOAT, 0, dod_colors);
    //glVertexPointer(3, GL_FLOAT, 0, vertices2);
	glVertexPointer(3, GL_FLOAT, 0, dod_vert);

    glPushMatrix();
    glTranslatef(-2, -2, 0);                // move to bottom-left corner

    //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);
	glDrawElements(GL_TRIANGLES, 108, GL_UNSIGNED_BYTE, dod_faces);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    //glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////
// draw cube at bottom-right corner with glDrawRangeElements()
// glDrawRangeElements() has two more parameters than glDrawElements() needs;
// start and end index. These values specifies a range of vertex data to be
// loaded into OpenGL. "start" param specifies where the range starts from, and
// "end" param specifies where the range ends. All the index values to be drawn
// must be between "start" and "end".
// Note that not all vertices in the range [start, end] will be referenced.
// But, if you specify a sparsely used range, it causes unnecessary process for
// many unused vertices in that range.
///////////////////////////////////////////////////////////////////////////////
void draw4()
{
    // enable and specify pointers to vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, normals2);
    glColorPointer(3, GL_FLOAT, 0, colors2);
    glVertexPointer(3, GL_FLOAT, 0, vertices2);

    glPushMatrix();
    glTranslatef(2, -2, 0);                 // move to bottom-right

    // draw first half (18 elements){0,1,2, 2,3,0, 4,5,6, 6,7,4, 8,9,10, 10,11,8}
    // The minimum index value in this range is 0, and the maximum index is 11,
    // therefore, "start" param is 0 and "end" param is 11.
    // Then, OpenGL will prefetch only 12 vertex data from the array prior to
    // rendering. (half of total data)
    glDrawRangeElements(GL_TRIANGLES, 0, 11, 18, GL_UNSIGNED_BYTE, indices);

    // draw last half (18 elements) {12,13,14, 14,15,12, 16,17,18, 18,19,16, 20,21,22, 22,23,20}
    // The minimum index value in this range is 12, and the maximum index is 23,
    // therefore, "start" param is 12 and "end" param is 23.
    // Then, OpenGL will prefetch only 12 vertex data from the array prior to
    // rendering.
    // Note that the last param of glDrawRangeElements(). It is the pointer to
    // the location of the first index value to be drawn.
    glDrawRangeElements(GL_TRIANGLES, 12, 23, 18, GL_UNSIGNED_BYTE, indices+18);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

///////////////////////////////////////////////////////////////////////////////
// draw cube at bottom-left corner with glDrawElements and interleave array
// All the vertex data (position, normal, colour) can be placed together into a 
// single array, and be interleaved like (VNCVNC...). The interleave vertex data
// provides better memory locality.
// Since we are using a single interleaved vertex array to store vertex
// positions, normals and colours, we need to specify "stride" and "pointer"
// parameters properly for glVertexPointer, glNormalPointer and glColorPointer.
// Each vertex has 9 elements of floats (3 position + 3 normal + 3 color), so,
// the stride param should be 36 (= 9 * 4 bytes).
///////////////////////////////////////////////////////////////////////////////
void draw5()
{
    // enable and specify pointers to vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 9 * sizeof(GLfloat), vertices3 + 3);
    glColorPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), vertices3 + 6);
    glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), vertices3);

    glPushMatrix();
    glTranslatef(-2, -2, 0);                // move to bottom-left

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}






///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // init global vars
    initSharedMem();

    // init GLUT and GL
    initGLUT(argc, argv);
    initGL();


    // check max of elements vertices and elements indices that your video card supports
    // Use these values to determine the range of glDrawRangeElements()
    // The constants are defined in glext.h
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxVertices);
    glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &maxIndices);

#ifdef _WIN32
    // get function pointer to glDrawRangeElements
    glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)wglGetProcAddress("glDrawRangeElements");
#endif
    
    // the last GLUT call (LOOP)
    // window will be shown and display callback is triggered by events
    // NOTE: this call never return main().
    glutMainLoop(); /* Start GLUT event-processing loop */

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// initialize GLUT for windowing
///////////////////////////////////////////////////////////////////////////////
int initGLUT(int argc, char **argv)
{
    // GLUT stuff for windowing
    // initialization openGL window.
    // it is called before any other GLUT routine
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);   // display mode

    glutInitWindowSize(screenWidth, screenHeight);  // window size

    glutInitWindowPosition(100, 100);               // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow(argv[0]);     // param is the title of window

    // register GLUT callback functions
    glutDisplayFunc(displayCB);
    glutTimerFunc(33, timerCB, 33);             // redraw only every given millisec
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);

    return handle;
}



///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void initGL()
{
    glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();
}



///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// draw a string in 3D space
///////////////////////////////////////////////////////////////////////////////
void drawString3D(const char *str, float pos[3], float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos3fv(pos);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initSharedMem()
{
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = mouseMiddleDown = false;
    mouseX = mouseY = 0;

    cameraAngleX = cameraAngleY = 0.0f;
    cameraDistance = CAMERA_DISTANCE;

    drawMode = 0; // 0:fill, 1: wireframe, 2:points
    maxVertices = maxIndices = 0;

    return true;
}



///////////////////////////////////////////////////////////////////////////////
// clean up global vars
///////////////////////////////////////////////////////////////////////////////
void clearSharedMem()
{
}



///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0, 0, 20, 1}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}



///////////////////////////////////////////////////////////////////////////////
// display info messages
///////////////////////////////////////////////////////////////////////////////
void showInfo()
{
    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    gluOrtho2D(0, screenWidth, 0, screenHeight); // set to orthogonal projection

    float color[4] = {1, 1, 1, 1};

    stringstream ss;
    ss << std::fixed << std::setprecision(3);

    ss << "Max Elements Vertices: " << maxVertices << ends;
    drawString(ss.str().c_str(), 1, screenHeight-TEXT_HEIGHT, color, font);
    ss.str("");

    ss << "Max Elements Indices: " << maxIndices << ends;
    drawString(ss.str().c_str(), 1, screenHeight-(2*TEXT_HEIGHT), color, font);
    ss.str("");

    // unset floating format
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

    // restore projection matrix
    glPopMatrix();                   // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
    glPopMatrix();                   // restore to previous modelview matrix
}



///////////////////////////////////////////////////////////////////////////////
// set projection matrix as orthogonal
///////////////////////////////////////////////////////////////////////////////
void toOrtho()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



///////////////////////////////////////////////////////////////////////////////
// set the projection matrix as perspective
///////////////////////////////////////////////////////////////////////////////
void toPerspective()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(screenWidth)/screenHeight, 1.0f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}









//=============================================================================
// CALLBACKS
//=============================================================================

void displayCB()
{
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();

    // tramsform camera
    glTranslatef(0, 0, -cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0);   // pitch
    glRotatef(cameraAngleY, 0, 1, 0);   // heading

    //draw1();        // with immediate mode, glBegin()-glEnd() block
    draw2();        // with glDrawArrys()
    draw3();        // with glDrawElements()
    //draw5();        // with glDrawElements() with interleave vertex array
    //draw4();        // with glDrawRangeElements()


    // print 2D text
    float pos[4] = {-4.0f,3.5f,0,1};
    float color[4] = {1,1,1,1};
    drawString3D("Immediate", pos, color, font);
    pos[0] = 0.5f;
    drawString3D("glDrawArrays()", pos, color, font);
    pos[0] = -5.0f; pos[1] = -4.0f;
    drawString3D("glDrawElements()", pos, color, font);
    pos[0] = 0.5f;
    drawString3D("glDrawRangeElements()", pos, color, font);


	///XXX
	if (0)
	{
		for (unsigned int i = 0;
			 i < sizeof(dod_vert)/sizeof(dod_vert[0]); i+=3) {
			float pos[4] = {0, 0, 0, 0};
			pos[0] = dod_vert[i + 0];
			pos[1] = dod_vert[i + 1];
			pos[2] = dod_vert[i + 2];
			std::stringstream s;
			s << "v" << int(i/3);
			drawString3D(s.str().c_str(), pos, color, font);
		}
	}


    showInfo();     // print max range of glDrawRangeElements

    glPopMatrix();

    glutSwapBuffers();
}


void reshapeCB(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    toPerspective();
}


void timerCB(int millisec)
{
    glutTimerFunc(millisec, timerCB, millisec);
    glutPostRedisplay();
}


void keyboardCB(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27: // ESCAPE
        clearSharedMem();
        exit(0);
        break;

    case 'd': // switch rendering modes (fill -> wire -> point)
    case 'D':
        ++drawMode;
        drawMode = drawMode % 3;
        if(drawMode == 0)        // fill mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
        else if(drawMode == 1)  // wireframe mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        else                    // point mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        break;

    default:
        ;
    }
}


void mouseCB(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if(button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if(state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if(button == GLUT_RIGHT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if(state == GLUT_UP)
            mouseRightDown = false;
    }

    else if(button == GLUT_MIDDLE_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if(state == GLUT_UP)
            mouseMiddleDown = false;
    }
}


void mouseMotionCB(int x, int y)
{
    if(mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown)
    {
        cameraDistance -= (y - mouseY) * 0.2f;
        mouseY = y;
    }
}
