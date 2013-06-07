/*
Use the nifty trick of hijacking the color buffer in the back plane as a pick buffer.
Make an array of pickable objects and render each as a different color on the integers (0..n-1).
Render everything else in the scene as white.
Now, when mouse is double clicked, get the color bit, calculate from this the offset into the
object array, and do something with that object (here, animate a rotation).
Be careful to check if all drawn objects' colors have been disabled in this draw routine.
Also NB! that if we change the main draw routine in main code, we better update it here. Must be a better way to manage this.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "my_hash.h"
#include "mySmallTreeLib.h"
#include "tree_openGL.h"
#include "picking.h"
#include "menu.h"



#define MAX_PICKED_OBJECTS 1000
#define COLOR_WHITE_AS_LONG_INT 16777215

struct PickableObject gObjectList[MAX_PICKED_OBJECTS];


float gTargetX, gTargetY;
GLubyte gObjectColorList[MAX_PICKED_OBJECTS][3];
GLubyte gColorID[3];
long gPickingIx;
node gCurrPickedNode;

//static void initPickableObject(struct PickableObject *p, enum ObjectType objType, OglTree t, node a, float height);
static void drawSceneTreePicking(OglTree T, int maxDepth);

static void drawTranslucentBox3dPick(float x, float y, float z);
static void drawFanPick(float x0, float y0, float z0, float r,  float theta1, float theta2);
static void initPicking(void);
static void displayPicking(void);
void drawTreePicking(OglTree T);
void drawNodePicking(OglTree currOglTree, node a);

// Hmm, waste of time: seems to do this even on a double click...
void handleSingleClick(int x, int y)
 {
	extern int gNumExpandedFanlets;
	extern OglTree gCurrFrontSubtree;
	long ix;
	 //node n;
	 // turn off texturing, lighting and fog
	 glDisable(GL_TEXTURE_2D);
	 glDisable(GL_FOG);
	 glDisable(GL_LIGHTING);
	 glDisable(GL_DITHER);
	 
	 initPicking();
	 glPushAttrib(GL_ALL_ATTRIB_BITS);
	 displayPicking();
	 glPopAttrib();
	 // get color information from frame buffer
	 GLubyte pixel[3];
	 
	 GLint viewport[4];
	 glGetIntegerv(GL_VIEWPORT, viewport);
	 
	 glReadPixels(x, viewport[3] - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
	 ix = 256*256*pixel[2] + 256*pixel[1] + pixel[0];

	 if (ix >= gPickingIx) 
		{
		if (ix == COLOR_WHITE_AS_LONG_INT)
			return;
		else
			printf("Warning: Read a pixel color (long ix = %li) out of bounds of picking buffer and not preset white bg color\n",ix);
		return;
		}

	if (gObjectList[ix].objType == FAN)
		gCurrFrontSubtree = gObjectList[ix].tree; // If clicking on a tree, make that tree come to front (mucks with depth buffer in tree_openGL.c)
	if (gObjectList[ix].objType == IMAGE)
		{
		gCurrFrontSubtree = NULL; // but if clicking on an image, don't make a tree come to front, stupid, and don't muck with depth buffer

		//extern float objTheta[3];
		//	objTheta[1] += 90;

		}
	return;
 }
 


void handleDoubleClick(int x, int y)

/*
Current behavior is that a double click on a root circle causes a rotation to bring that into
foreground. A double click on a nested circle, causes its collapse to a fanlet. A double click
on a fanlet causes expansion.
*/

 {
	extern int gNumExpandedFanlets;
	extern OglTree gCurrOglSubtree;
	long ix;
	 //node n;
	 // turn off texturing, lighting and fog
	 

	 glDisable(GL_TEXTURE_2D);
	 glDisable(GL_FOG);
	 glDisable(GL_LIGHTING);
	 glDisable(GL_DITHER);
	 
	 initPicking();
	 glPushAttrib(GL_ALL_ATTRIB_BITS);
	 displayPicking();
	 glPopAttrib();
	 
	 // get color information from frame buffer
	 GLubyte pixel[3];
	 
	 GLint viewport[4];
	 glGetIntegerv(GL_VIEWPORT, viewport);
	 
	 glReadPixels(x, viewport[3] - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
	 
	 ix = 256*256*pixel[2] + 256*pixel[1] + pixel[0];
	 if (ix >= gPickingIx) 
		{
		if (ix == COLOR_WHITE_AS_LONG_INT)
			return;
		else
			printf("Warning: Read a pixel color (long ix = %li) out of bounds of picking buffer and not preset white bg color\n",ix);
		return;
		}
	 OglTree t	= gObjectList[ix].tree;
	 node a		= gObjectList[ix].nd;
	 float y_off= gObjectList[ix].height;
	 
	gCurrOglSubtree = t;
	gCurrPickedNode = a; // careful; this is sometimes assigned NULL, so use it responsibly
	
	extern int animStepsFanlet,gAnimateDirection;

// The next two clauses expand or contract a fanlet; otherwise go ahead and do some kind of rotation on 


	
	switch (gObjectList[ix].objType)
		{
		case FAN:
			// this first pair of clauses takes care of fanlet fans being picked, which expand or contract the fanlet before rotations
			if (t->defaultLayout == fanlet && t->layout == fanlet)
				{
				fanletAnimation(t,+1);
				return;
				}
			else if (t->defaultLayout == fanlet && t->layout== circle)
				{
				fanletAnimation(t,-1);
				return;
				}
			// else fall through and just do a rotation of the fan
		case IMAGE:
		case LABEL:
			animateRotationToObject(&gObjectList[ix]);
	
	
		}
	return;
 }
 
 


static void initPicking(void)
	{
	int i;
	gPickingIx = 0;
	for (i=0;i<3;i++)
		gColorID[i]= 0;
	}

// Increments the color index, stores it in the global object array, stores the corresponding node of the object, and sets glColor to this value for picking purposes.

void colorAndRegisterObject(enum ObjectType objType, OglTree t, node a, float height) 
	// If object is a tree, pass NULL for the node; if it's an image or a label, pass the node and the tree to which it belongs;
	// height is height of the object
	{
	int i;
	if (gPickingIx >= MAX_PICKED_OBJECTS)
		{
		printf("Warning: Pick buffer full: Cannot add more objects\n");
		return;
		}
//	gObjectList[gPickingIx].tree=t ; 
//	gObjectList[gPickingIx].nd= a; 
//	gObjectList[gPickingIx].height= height; 
//	gObjectList[gPickingIx].objType= objType; 

	initPickableObject(&gObjectList[gPickingIx],objType, t, a, height);

//printf("Object %li registered\n",gPickingIx);
	for (i=0;i<3;i++)
		gObjectColorList[gPickingIx][i]= gColorID[i];
	
	glColor3ubv(gColorID);
//printf("%li %s %i %i %i\n",gPickingIx,t->root->label,gColorID[2],gColorID[1],gColorID[0]);
	
	gColorID[0]++;
	if(gColorID[0] == 0)	// nudges over to next place when back to 0; remember GLubyte = [0,255]
	   {
		gColorID[1]++;
		if(gColorID[1] == 0)
			{
			 gColorID[2]++;
			}
	   }
	++gPickingIx;
	
	
	return;
	}
 
 void initPickableObject(struct PickableObject *p, enum ObjectType objType, OglTree t, node a, float height)
	{
	p->objType=objType;
	p->tree=t;
	p->nd=a;
	p->height=height;
	return;
	
	}
 
 static void displayPicking(void)
 
 // Use the color buffer for picking.
 // Set everything in the scene that is not registered as a pickable object to white (1,1,1), then test for that.
 
{
	extern int gRenderBin,gNumRenderBins;
	extern float theta[3];
	extern GLfloat bgColor[4];
	extern OglTree gCurrOglTree;
	glClearColor(1,1,1,1);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();


// ..translate if necessary and then rotate and only then draw the scene
	glTranslatef(-gP->X,-gP->Y,-gP->Z);	// shift mouse controlled translations of camera (tree)

	glRotatef(theta[0],1.0,0.0,0.0);	// mouse controlled rotations around the tree's center...
	glRotatef(theta[1],0.0,1.0,0.0);
	glRotatef(theta[2],0.0,0.0,1.0);



// Construct the scene

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	// now actually draw the tree with the next routine


	for (gRenderBin=1; gRenderBin<=gNumRenderBins; gRenderBin++)
		drawSceneTree(PICKING, gCurrOglTree, MAX_SCENE_TREE_DEPTH);
		
	glFlush();
	//glutSwapBuffers(); // 
  	glClearColor (bgColor[0], bgColor[1],bgColor[2],bgColor[3]);
	}


 