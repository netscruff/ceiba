#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
  #include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include "layout.h"

#define glutStrokeLength(x,y) myFTStringLength(y)

#define MAX_SCENE_TREE_DEPTH 5  //  display this many levels in the tree
#define BOXW 7.0	// terminal bounding boxes for images, etc.

enum DrawEnum_ {RENDERING, PICKING};
typedef enum DrawEnum_ DrawEnum;

void animateBoxRotate(void);
void drawSquare(float width);
void fanletAnimation(OglTree t, short direction);
OglTree findRootCircle(OglTree T);
short inCircularInterval(float minX, float maxX, float x);
float clamp0_TwoPI(float angle);
float clamp0_360(float angle);
void animateFanlet(void);
void setupTransformations(OglTree T, int maxDepth);
void drawSceneTree(DrawEnum render, OglTree T, int maxDepth);
void drawFan(DrawEnum render, float x0, float y0, float z0, float r,  float theta1, float theta2, float opacity, short doAttenuateBoundingShape, GLfloat fanColor[]  );
void drawDiagBitmap(int width, GLubyte * bitmap);
void plane_update(void);
void spinDisplay(float angle);
void replace_underscore(char *p);
void drawCylinderArc(double theta1, double r1, double z1, double theta2, double r2, double z2,  float radius, enum BranchStyle branchStyle);
void drawCylinders(double *v, int numpts, float radius, enum BranchStyle branchStyle);
void drawCylinder(double x1, double y1, double z1, double x2, double y2, double z2, float radius1, float radius2);
int tree_openGL(int argc, char** argv, OglTree tree);
void cylinders(node a);
void setup_cylinders_trig(OglTree t, node a);
void drawNode(DrawEnum render, OglTree t, node a);
void strtoupper(char *s);
char * firstalpha(char *s);
int nodeLabelCompar(const void *elem1, const void * elem2);
void drawTranslucentBox(float width, float height, float z_offset, float opacity);
void drawTranslucentBox3d(DrawEnum render, float x, float y, float z, float opacity);
void animateTwist(void);


typedef struct planeStruct * plane;
struct planeStruct 
	{
	float X;
	float Y;
	float Z;   // position

	float Roll;
	float Pitch;
	float Pitch0;
	float Heading;
	float Heading0;

	float Xdir;
	float Ydir;
	float Zdir;  // vector version of heading...

	float step;  // distance moved in one step
	float speed;
	float angular_speed; // degrees rotated in one step
	};
plane gP;

#define RC (2*3.141592/360)
#define toRads(x) ((x)*RC)
#define toDegs(x) ((x)/RC)

