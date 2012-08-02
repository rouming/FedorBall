///////////////////////////////////////////////////////////////////////////////
// main.cpp
// ========
// testing vertex array (glDrawElements, glDrawArrays)
//
//	AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2005-10-04
// UPDATED: 2012-04-11
///////////////////////////////////////////////////////////////////////////////

#include <GL/glut.h>
#include <GL/glext.h>

// glm::vec3, glm::vec4, glm::ivec4, glm::mat4
#include <glm/glm.hpp>
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/matrix_transform.hpp>
// glm::value_ptr
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <list>

#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "raytri.h"

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
int	 initGLUT(int argc, char **argv);
bool initSharedMem();
void clearSharedMem();
void initLights();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
void drawString(const char *str, int x, int y, float color[4], void *font);
void drawString3D(const char *str, float pos[3], float color[4], void *font);
void toOrtho();
void toPerspective();
void drawDodecahedron();


// constants
const int	SCREEN_WIDTH	= 400;
const int	SCREEN_HEIGHT	= 300;
const float CAMERA_DISTANCE = 4.0f;
const int	TEXT_WIDTH		= 8;
const int	TEXT_HEIGHT		= 13;


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

#define pi 3.14159265358

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

// (1.0 + sqrt(5.0)) / 2.0;
static float phi = 1.61803;
static float phi_1 = 1-phi;
static GLfloat dod_orig_vert[] =
{
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
	12, 2, 0,	12, 0, 14,	12, 14, 5,
	10, 8, 16,	10, 16, 2,	10, 2, 12,
	17, 6, 7,	17, 7, 16,	17, 16, 8,
	16, 7, 18,	16, 18, 0,	16, 0, 2,
	0,	18, 9,	0,	9, 11,	0, 11, 14,
	5, 14, 11,	5, 11, 15,	5, 15, 4,
	13, 4, 15,	13, 15, 1,	13, 1, 3,
	17, 3, 1,	17, 1, 19,	17, 19, 6,

	10, 12, 5,	10, 5, 4,	10, 4, 13,
	3, 17, 8,	3, 8 ,10,	3, 10, 13,
	11, 9, 19,	11, 19, 1,	11, 1, 15,
	9, 18, 7,	9, 7, 6,	9, 6, 19,
};

//faces * verteces_per_face * cords_in_3d
static GLfloat dod_vert[12*5*3];
//faces * verteces_per_face * cords_in_3d
static GLfloat dod_normals[12*5*3];
//faces * triangles_per_face * points_per_triangle
static GLbyte dod_faces[12*3*3];

void drawDodecahedron()
{
	float d = cameraDistance;
	float alpha = cameraAngleY*pi/180;
	float beta = cameraAngleX*pi/180;

	float t = d*cos(alpha);
	float y = d*sin(alpha);
	float x = t*cos(beta);
	float z = t*sin(beta);

	point line_dir = {x, y, z};

	//XXX
	static int s_inited = 0;
	if (!s_inited)
	{
		s_inited = 1;

		// NORMALIZE AND ROTATE
		if (1)
		{
			//  atan(1/phi) * (180/pi),
			//  www.trinitas.ru/rus/doc/0232/006a/02321014.pdf
			float angle = 31.717f;
			glm::mat4 rot = glm::rotate(glm::mat4(1.0), angle,
										glm::vec3(1.0f, 0.0f, 0.0f));

			printf("const static fp_t s_coords_vert[] =\n{\n");
			for (unsigned int i = 0;
				 i < sizeof(dod_orig_vert)/sizeof(dod_orig_vert[0]);
				 i += 3)
			{
				GLfloat x = dod_orig_vert[i+0];
				GLfloat y = dod_orig_vert[i+1];
				GLfloat z = dod_orig_vert[i+2];

				glm::vec3 vec3 = glm::normalize(glm::vec3(x, y, z));
				glm::vec4 vec = rot * glm::vec4(vec3, 0.0f);

				dod_orig_vert[i+0] = vec.x;
				dod_orig_vert[i+1] = vec.y;
				dod_orig_vert[i+2] = vec.z;

				printf("\t%hd,\t\t%hd,\t\t%hd,\n",
					   FTOFP(vec.x), FTOFP(vec.y), FTOFP(vec.z));
			}
			printf("};\n");
		}

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
				vector v = {x, y, z};
				point p = {v.x, v.y, v.z};
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
	glEnableClientState(GL_VERTEX_ARRAY);
	glNormalPointer(GL_FLOAT, 0, dod_normals);
	glVertexPointer(3, GL_FLOAT, 0, dod_vert);

	glPushMatrix();
	glTranslatef(0, 0, 0);

	glDrawElements(GL_TRIANGLES, 108, GL_UNSIGNED_BYTE, dod_faces);

	glPopMatrix();

	// Draw lines
	glBegin(GL_LINES);
	glColor3f(1,0,0);
	glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
	glVertex3f(line_dir.x, line_dir.y, line_dir.z); // ending point of the line
	glColor3f(1,1,1);
	glEnd();

	// Draw sphere
	glColor3f(0,1,0);
	//glutSolidSphere(0.96999, 100, 100);
	glColor3f(1,1,1);

	// Draw index of every vertex
	{
		for (int face = 0; face < 12; ++face) {
			for (int vert = 0; vert < 5; ++vert) {
				// we should get last 2 vertex from faces array
				if (vert == 3 || vert == 4)
					vert += 4;

				int v = dod_orig_faces[face*9 + vert];

				float pos[3] = {dod_orig_vert[v*3 + 0],
								dod_orig_vert[v*3 + 1],
								dod_orig_vert[v*3 + 2]};
				float color[4] = {0,0,1,1};
				std::stringstream s;
				s << v;
				drawString3D(s.str().c_str(), pos, color, font);
			}
		}
	}

	// Draw intersections
	{
		int sz = sizeof(dod_orig_faces)/sizeof(dod_orig_faces[0]);
		for (int i = 0; i < sz; i += 3) {
			triangle tr;
			create_triangle(dod_orig_vert, dod_orig_faces, i, tr);

			//Count intersection with triangle
			fp_t orig[3] = {0, 0, 0};
			fp_t dir[3] = {FTOFP(line_dir.x),
						   FTOFP(line_dir.y),
						   FTOFP(line_dir.z)};
			fp_t vert0[3] = {FTOFP(tr.p0.x),
							 FTOFP(tr.p0.y),
							 FTOFP(tr.p0.z)};
			fp_t vert1[3] = {FTOFP(tr.p1.x),
							 FTOFP(tr.p1.y),
							 FTOFP(tr.p1.z)};
			fp_t vert2[3] = {FTOFP(tr.p2.x),
							 FTOFP(tr.p2.y),
							 FTOFP(tr.p2.z)};
			fp_t t, u, v;
			if (fixed_intersect_triangle(orig, dir,
										 vert0, vert1, vert2,
										 &t, &u, &v) && t > 0) {

				// Draw lines
				glBegin(GL_LINES);
				glColor3f(0,1,1);
				glVertex3f(tr.p0.x, tr.p0.y, tr.p0.z);
				glVertex3f(tr.p1.x, tr.p1.y, tr.p1.z);

				glVertex3f(tr.p1.x, tr.p1.y, tr.p1.z);
				glVertex3f(tr.p2.x, tr.p2.y, tr.p2.z);


				glVertex3f(tr.p2.x, tr.p2.y, tr.p2.z);
				glVertex3f(tr.p0.x, tr.p0.y, tr.p0.z);

				glColor3f(1,1,1);
				glEnd();

			}
		}
	}


	glDisableClientState(GL_VERTEX_ARRAY);	// disable vertex arrays
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

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);	// display mode

	glutInitWindowSize(screenWidth, screenHeight);	// window size

	glutInitWindowPosition(100, 100);				// window location

	// finally, create a window with openGL context
	// Window will not displayed until glutMainLoop() is called
	// it returns a unique ID
	int handle = glutCreateWindow(argv[0]);		// param is the title of window

	// register GLUT callback functions
	glutDisplayFunc(displayCB);
	glutTimerFunc(33, timerCB, 33);				// redraw only every given millisec
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
	glShadeModel(GL_SMOOTH);					// shading mathod: GL_SMOOTH or GL_FLAT
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);		// 4-byte pixel alignment

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

	glClearColor(0, 0, 0, 0);					// background color
	glClearStencil(0);							// clear stencil buffer
	glClearDepth(1.0f);							// 0 is near, 1 is far
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
	glDisable(GL_LIGHTING);		// need to disable lighting for proper text color
	glDisable(GL_TEXTURE_2D);

	glColor4fv(color);			// set text color
	glRasterPos2i(x, y);		// place text position

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
	glDisable(GL_LIGHTING);		// need to disable lighting for proper text color
	glDisable(GL_TEXTURE_2D);

	glColor4fv(color);			// set text color
	glRasterPos3fv(pos);		// place text position

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
	GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};	// ambient light
	GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};	// diffuse light
	GLfloat lightKs[] = {1, 1, 1, 1};			// specular light
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

	// position the light
	float lightPos[4] = {0, 0, 20, 1}; // positional light
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	glEnable(GL_LIGHT0);						// MUST enable each light source after configuration
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
	glPushMatrix();						// save current modelview matrix
	glLoadIdentity();					// reset modelview matrix

	// set to 2D orthogonal projection
	glMatrixMode(GL_PROJECTION);		// switch to projection matrix
	glPushMatrix();						// save current projection matrix
	glLoadIdentity();					// reset projection matrix
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
	glPopMatrix();					 // restore to previous projection matrix

	// restore modelview matrix
	glMatrixMode(GL_MODELVIEW);		 // switch to modelview matrix
	glPopMatrix();					 // restore to previous modelview matrix
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
	glRotatef(cameraAngleX, 1, 0, 0);	// pitch
	glRotatef(cameraAngleY, 0, 1, 0);	// heading

	drawDodecahedron();		   // with glDrawElements()

	///XXX
	if (0)
	{
		// print 2D text
		//float pos[4] = {-4.0f,3.5f,0,1};
		float color[4] = {1,1,1,1};

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


	showInfo();		// print max range of glDrawRangeElements

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
		if(drawMode == 0)		 // fill mode
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
		}
		else if(drawMode == 1)	// wireframe mode
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
		}
		else					// point mode
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
