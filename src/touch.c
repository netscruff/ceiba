#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "my_hash.h"
#include "mySmallTreeLib.h"
#include "tree_openGL.h"
#include "picking.h"
#include <time.h>

#define INIT_SLOW_MOVE	0.1	// initial speed for slow motion controls
#define INIT_SLOW_ANGLE 1.0	// initial angular movement in degrees for slow mo controls
#define INIT_SPEED 0.1  // this is the initial speed used when switching to flight mode
#define INIT_ACTIVE_ALT 90 // this is the angular movement in degrees when an alt-arrow is used

int trackingMouse=0;
int gShift=0;
GLenum gKeyModifier;
float lastY,lastX,startX,startY;
float gMoveFactor=0.2; // adjust move amounts for translations
float gMoveFactor1=0.3; // adjusts move amounts for rotations
float gSlowAngle=INIT_SLOW_ANGLE;
float gSlowMove=INIT_SLOW_MOVE;

// ** HANDLE INPUT FROM KEYBOARD NORMAL KEYS **

void keyboard(unsigned char key, int x, int y)
{
extern float gRootRadius, gLeafRadius;
extern int gLeafRotate;
static int ksubn=0;
extern Hash subclades;
Entry e;
node subn;
extern int gSqueezeLayout,gLabelFace,sphereMode,gCorners,gSuppressTerminals,fullScreen,gSpinTree,init_window_width,init_window_height;
extern float gr1,gr2,gz1,gz2,gLabelAttenuate,cylinderRadius,gRootRadius, gLeafRadius, gArcOffset,gArcLabelOffset,gLineWidthArcName,gLineWidthLabel,gLineWidthTree,multLabelScale,labelScale,intLabelScale,init_line_width_label,init_line_width_tree,baseLabelScale,gSlowSpinAngle, gBoxWidth;
extern node curNode;
extern node gTreeRoot,gn1,gBaseTreeRoot;
extern double gRadiusOfOriginalCircle;
area A;
   switch (key) 
	{
      	case 27:  // escape key
        	 exit(0);
        	 break;
	case 'f':if (fullScreen) 
			{
			fullScreen = 0; 
			glutReshapeWindow(init_window_width, init_window_height);
			}
		 else 
			{
			fullScreen=1;
			glutFullScreen();
			}
		 break;
	case '1':gr1+=0.1;break;  // position of bezier control points...
	case '!':gr1-=0.1;break;
	case '2':gz1+=0.1;break;
	case '@':gz1-=0.1;break;
	case '3':gr2+=0.1;break;
	case '#':gr2-=0.1;break;
	case '4':gz2+=0.1;break;
	case '$':gz2-=0.1;break;
	case '8':gLabelAttenuate += 1; ;break;
	case '9':gLabelAttenuate -= 1; if (gLabelAttenuate <= 0)gLabelAttenuate=1; break;
	case 'e':gP->speed=0.0;break;
	case 'E':gP->speed=INIT_SPEED;break;
	case 'o':gSpinTree=0;break;
	case 'O':gSpinTree=1;break;
	case '<':gP->speed/=1.3;break;
	case '>':gP->speed*=1.3;break;
	case '{':gSlowAngle/=1.3;break;
	case '}':gSlowAngle*=1.3;break;
	case '[':gSlowSpinAngle/=1.3;break;
	case ']':gSlowSpinAngle*=1.3;break;
	case 'c':cylinderRadius/=1.1; gRootRadius/=1.1; setRadius(gTreeRoot, gTreeRoot, gRootRadius, gLeafRadius); break;
	case 'C':cylinderRadius*=1.1; gRootRadius*=1.1; setRadius(gTreeRoot, gTreeRoot, gRootRadius, gLeafRadius); break;
	case 'U':gLineWidthLabel+=0.25;break;  // for label text printing
	case 'u':gLineWidthLabel-=0.25; ;break;
	case 'j':gLeafRotate=0;break;  // toggle rotations of leaf object instead of scene
	case 'J':gLeafRotate=1; ;break;
	case 'V':gLineWidthTree+=1.0;break;  // for label text printing
	case 'v':gLineWidthTree-=1.0; break;
	case 'l':multLabelScale/=1.1; break; // for horizontal labels only
	case 'L':multLabelScale*=1.1; break;
	case 'W': gBoxWidth*=1.1; break;
	case 'w': gBoxWidth/=1.1; break;
	case 's': if (sphereMode) sphereMode=0; else sphereMode=1; break;
	case 'S': if (gCorners) gCorners=0; else gCorners=1; 
		  //setupTheTree(gTreeRoot); CURRENTLY DISABLING THIS; NEED TO REPLACE WITH oglTreeInit
		  break;
	case 'g': if (gSqueezeLayout>1) --gSqueezeLayout; // move nodes out toward tips more and more
		  //setupTheTree(gTreeRoot);
		  break;
	case 'G': ++gSqueezeLayout; 
		  //setupTheTree(gTreeRoot);
		  break;
       case 'x':gP->X -= gP->step; break;
       case 'X':gP->X += gP->step; break;
       case 'y':gP->Y -= gP->step; break;
       case 'Y':gP->Y += gP->step; break;
       case 'z':gP->Z -= gP->step; break;
       case 'Z':gP->Z += gP->step; break;


	case 'T':
		if (gSuppressTerminals) gSuppressTerminals=0; else gSuppressTerminals=1; break;

	case 't':
		if (subclades)
			{
			e= hashGetKthEntry(subclades,ksubn++);
			if (e)
				{
				subn = e->val;
				//setupTheTree(subn);
				gTreeRoot=subn;
				}
			else
				{
				ksubn=0;
				gTreeRoot=gBaseTreeRoot;
				}
			}
		break;
   	}
plane_update();
glutPostRedisplay();
}


// ** HANDLE THE SLOW MOTION CONTROLS USING ARROW KEYS ** 

void specialKeyboard(int key, int x, int y)
{
extern int boxAnimSteps; extern int arrayIndex;
extern int animIndex;
extern double boxThetaStep;
extern float objTheta[3];
extern int axis;
extern float zStep,thetaStep,xStep,theta0step;
extern int animSteps;
int modifier,dir;
modifier=glutGetModifiers();
if (modifier ==  GLUT_ACTIVE_ALT)
   { 
   switch (key) 
      {
      case GLUT_KEY_UP:
		arrayIndex=0;
		dir = -1;	
         break;
      case GLUT_KEY_DOWN:
		arrayIndex=0;
		dir = +1;	
         break;
      case GLUT_KEY_LEFT:
		arrayIndex=1;
		dir = -1;	
		break;
      case GLUT_KEY_RIGHT:
		arrayIndex=1;
		dir = +1;	
		break;
      }
	boxAnimSteps = 20;
//	animIndex=boxAnimSteps;
	boxThetaStep = dir * 90.0/ boxAnimSteps;
	glutIdleFunc(animateBoxRotate);
   }
else
if (modifier ==  GLUT_ACTIVE_SHIFT)
   { 
   switch (key) 
      {
      case GLUT_KEY_UP:
	gP->Z -= gSlowMove;
	glutPostRedisplay();
         break;
      case GLUT_KEY_DOWN:
	gP->Z += gSlowMove;
	glutPostRedisplay();
         break;
      case GLUT_KEY_LEFT:
	gP->X -= gSlowMove;
	glutPostRedisplay();
         break;
      case GLUT_KEY_RIGHT:
	gP->X += gSlowMove;
	glutPostRedisplay();
         break;
      }
   }
/***/
else
if (modifier ==  GLUT_ACTIVE_CTRL)
   { 
   switch (key) 
      {
      case GLUT_KEY_UP:
	gP->Pitch += gSlowAngle;
	glutPostRedisplay();
         break;
      case GLUT_KEY_DOWN:
	gP->Pitch -= gSlowAngle;
	glutPostRedisplay();
         break;
     case GLUT_KEY_LEFT:
	gP->Heading += gSlowAngle;
//        axis=1; spinDisplay(gSlowAngle);
        glutPostRedisplay();
         break;
      case GLUT_KEY_RIGHT:
	gP->Heading -= gSlowAngle;
//        axis=1; spinDisplay(-gSlowAngle);
        glutPostRedisplay();
         break;
 
      }
   }
/***/
else
   { 
   switch (key) 
      {
      case GLUT_KEY_UP:
	axis=0; spinDisplay(-gSlowAngle);
	glutPostRedisplay();
         break;
      case GLUT_KEY_DOWN:
	axis=0; spinDisplay(+gSlowAngle);
	glutPostRedisplay();
         break;
      case GLUT_KEY_LEFT:
	axis=2; spinDisplay(gSlowAngle);
	glutPostRedisplay();
         break;
      case GLUT_KEY_RIGHT:
	axis=2; spinDisplay(-gSlowAngle);
	glutPostRedisplay();
         break;
      }
   }
}



void startMotion(int x, int y);
void stopMotion(int x, int y);
void mouseMotion(int x, int y);

// ** Handles mouse button clicks 
#define DOUBLE_CLICK_TIME .250 // milliseconds

int gWaitingForAnotherMouseClick=0;
static void dbl_click_timer(int value)
	{
	gWaitingForAnotherMouseClick = 0;
	}

void mouse2(int button, int state, int x, int y) 
{
int modifier;
modifier=glutGetModifiers();
if (modifier ==  GLUT_ACTIVE_SHIFT) gShift = 1; else gShift=0; 
   switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state==GLUT_DOWN)
			{
			if (gWaitingForAnotherMouseClick)
				handleDoubleClick(x,y);
			else
				{
				gWaitingForAnotherMouseClick=1;
				glutTimerFunc(DOUBLE_CLICK_TIME,dbl_click_timer,1); // last arg not currently used
				startMotion(x,y);
				}
			
			}
		if (state==GLUT_UP)
			{
			stopMotion(x,y);
			if (!gWaitingForAnotherMouseClick && x==startX && y==startY)
				printf("Single click no move\n");
			}
		break;
	default:
		break;
	}
}
#define SINGLE_CLICK_TIME .250
	
int gWaitingForMouseUp=0;
int gClickOccurred=0;
static void sngl_click_timer(int value)
	{
	gWaitingForMouseUp = 0;
	}


long tLastDown, tLastClick;
void mouse3(int button, int state, int x, int y) 
	{
	/*
	Christ, too much trouble with glut timers. Just use C clock() function!

	*/
	int modifier;
	long time;
	modifier=glutGetModifiers();
	if (modifier ==  GLUT_ACTIVE_SHIFT) gShift = 1; else gShift=0; 
	   switch (button)
		{
		case GLUT_LEFT_BUTTON:
			if (state==GLUT_DOWN)
				{
				tLastDown = clock();
				//printf("time %f\n",(float)clock()/CLOCKS_PER_SEC);			
				startMotion(x,y);
				}
			if (state==GLUT_UP)
				{
				time = clock();
				if (  (float)(time - tLastDown)/CLOCKS_PER_SEC < SINGLE_CLICK_TIME ) // we have a single click
					{
					//printf ("SINGLE CLICK\n");
//printf ("Time since last click %f\n", (float)(time - tLastClick)/CLOCKS_PER_SEC);
					if (  (float)(time - tLastClick)/CLOCKS_PER_SEC < DOUBLE_CLICK_TIME    ) 
						{
						//printf ("DOUBLE CLICK\n");
						handleDoubleClick(x,y);
						}
					else
						{
						//printf ("SINGLE CLICK ONLY!\n");
						handleSingleClick(x,y);
						}
					tLastClick = time;
					}
				stopMotion(x,y);
				}
			break;
		default:
			break;
		}
		
		
	}


	

void mouse(int button, int state, int x, int y) // NOT USED
{
int modifier;
gShift=glutGetModifiers();
   switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state==GLUT_DOWN)
			startMotion(x,y);
		if (state==GLUT_UP)
			stopMotion(x,y);
		break;
	default:
		break;
	}
}
// ** Handles mouse movement while buttons are pressed: *all* movement is done only while mouse buttons are pressed 
// Move in the screen y direction causes a rotation about the x coord axis.
// Move in the screen x direction causes a rotation about the z coord axis.
// But note that the default initial view of the tree has the x axis upward toward top of the monitor


void mouseMotion(int x, int y) 
	{
	extern int gLeafRotate;
	extern float objTheta[3];
	extern float theta[3];
       float dy,dx;
	if (trackingMouse)  
	    {
	    dy = y - lastY;
	    lastY = y;	
	    dx = x - lastX;
	    lastX = x;	

	    if (gShift & GLUT_ACTIVE_SHIFT)  // case of shift click and move ; ...translate
	    	{
			gP->Z += gMoveFactor*dy;
			gP->X += gMoveFactor*dx;
	    	}
	    else // case of just plain click and move ; ...rotate whole scene or just the leaf object depending on flag
			if (gLeafRotate == 0)
				{
				theta[0]+=dy*gMoveFactor1;
				theta[2]-=dx*gMoveFactor1;
				if (theta[0]>360.0)
					theta[0]-=360.0;
				if (theta[0]<0.0)
					theta[0]+=360.0;
				if (theta[2]>360.0)
					theta[2]-=360.0;
				if (theta[2]<0.0)
					theta[2]+=360.0;
				}
			else
				{
				objTheta[0]+=dy*gMoveFactor1;
				objTheta[2]-=dx*gMoveFactor1;
				}
			
 	    }
	glutPostRedisplay();
	}
void startMotion(int x, int y)
	{
	trackingMouse = 1;
	startX = x; startY = y;
	lastX = x; lastY = y;
	}
void stopMotion(int x, int y)
	{
	trackingMouse = 0;
	}


