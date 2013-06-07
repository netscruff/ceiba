/****

January 2009. Added cylinder drawing using precomputed trig (which I had already precomputed in early versions!).

As of November 8, 2007, I've really hacked into this code, changed the mySmallTreeLib so that I can have newick trees with TWO float values at each node. Then when BOXES is set to 1, we display boxes for each terminal in which thee brightness is determined by one value and the size of the box by the other.

In the nexus file, these are indicated as ...bob):1.333:0.004, betty:1.2:3.5), etc....

****/
//#include <GLUT/glut.h> ...defined in layout_cone.h
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "my_hash.h"
#include "mySmallTreeLib.h"
//#include "layout.h"
#include "tree_openGL.h"
#include "tr.h"
#include "getPPM.h" 
#include "getSTL.h"
#include "getXYZ.h"
#include "touch.h"
#include "frustum_cull.h"
#include "branch.h"
#include "picking.h"
#include "fonts.h"

//#include <tiffio.h>     /* Sam Leffler's libtiff library. */



// ********** FOLLOWING IS KEY SET OF HARD CODED OPTIONS FOR UNUSUAL CIRCUMSTANCES **************

#define WALL  0		// set to 1 to use on the tile wall (disables the menu cascade)
#define TILES 0		// set to 1 to use tile mode and write tiled image to file
#define BOXES 0		// set to 1 to display informative and colored rectangles in circular layout
#define TICKS 0		// display character state changes stored as :x:y in tree desc as tick boxes
#define LEAF_STATES 0 // display leaf states from 'y' above as colored boxes

/******************** DEFAULT STATES OF IMPORTANT PARAMETERS THAT CAN BE CHANGED BY USER *******************/

//GLfloat bgColor[4]={0.7,0.7,0.7,1.0}; // color of sky/background ; good for printing
GLfloat bgColor[4]={0.33,0.33,0.33,1.0}; // color of sky/background ; good for screen

#define NURBS 1
#define PERSPECTIVE 1.0		// Increase this to widen field of view to make it more fisheye (1 is a good default); used in glFrustum
#define FRONTCLIP 1.5		// Can drop this to ~ 0.5 but gets more distortion that way...
#define BACKCLIP 500		// 1.5 - 1000 has been working

#define G_CORNERS		0
#define INIT_GSUPPRESSLABELS 	0
#define INIT_GLABELFACE 	1
#define LABEL_SCALE 		0.003 
#define FULLSCREEN 		1
#define SPHERE 			0 		// makes little spheres where the corners of cladogram are
#define CYL_RADIUS 		0.05

/******************** PARAMETERS CONTROLLING SCENE NOT LIKELY TO CHANGE DURING RUN **************************/

#define IMAGE_OFFSET_Y 50 // set image this far up from taxon label
#define CYLS 1		// set to 1 for 3D cylinders for branches, otherwise gumby lines
					// Mucked up; includes hard coded green tree material...
#define ROOT_LENGTH 100.0 

#define INITGTEXBOXSIZE 400	// Size of image box

#define SPARSE_LABELS	0	// Set to 1 to display only some of the labels
#define INIT_Z_FACTOR 1.25		// viewer is initially at +INIT_Z_FACTOR*gRadiusOfOriginalCircle
//#define INIT_Z_FACTOR .65		// viewer is initially at +INIT_Z_FACTOR*gRadiusOfOriginalCircle
#define CASCADE_TO_DISTANCE 2.0	// when select terminal from cascade list, zoom to this distance along +z-axis

#define INTLABELFACTOR 15.0	// internal labels are this times as large as terminal labels
#define ARCOFFSETFACTOR 0.45	// initial factor for gArcOffset
#define ARC_RADIUS 0.20		// radius of cylinders in arcs labelling large clades
//#define TERMOFFSETFACTOR 0.02	...at the moment, I don't like how this scales to very large trees->puts them too far from tips
#define TERMOFFSETFACTOR 0.01	
#define ARCLABELOFFSETFACTOR 0.080
#define INTOFFSETFACTOR 

#define INIT_WINDOW_WIDTH 1000
#define INIT_WINDOW_HEIGHT 800
#define INIT_WINDOW_POS_X 100
#define INIT_WINDOW_POS_Y 100

int init_window_width = INIT_WINDOW_WIDTH, init_window_height = INIT_WINDOW_HEIGHT;

#define GROUND_PLANE_SIZE 1000


float gSlowSpinAngle= 1.0; //for spinning tree during animation


// ************************ NURBS stuff.... ***********************************************************


float gr1,gr2,gz1,gz2;  // used to modify two control points in NURBS in drawCylinderPrecomputed


// ************************ DEALING WITH TILED IMAGES .... ***********************************************************

#if BOXES
#define CIRCLE_SCALE 1
#else
#define CIRCLE_SCALE 0
#endif

#define TILE_WIDTH 512
#define TILE_HEIGHT 512
#define TILE_BORDER 10
#if 0
#define IMAGE_WIDTH 2048
#define IMAGE_HEIGHT 1536	// mult both x4 to get up to biggest practical size... 
#else
#define IMAGE_WIDTH 8192
#define IMAGE_HEIGHT 6144	// mult both x4 to get up to biggest practical size... 
#endif
#define FILENAME "tileimg.ppm"
#if TILES
#define INITLINEWIDTHLABEL 2.5		// for tiled display have to make lines thicker...
#define INITLINEWIDTHTREE 3.0
#define INITLINEWIDTHARC 18.0	// line width of the arc for MRCA names (not for the name itself)
#define INITLINEWIDTHARCNAME 8.0	
#define LINEWIDTHCIRCLESCALE 3.0		
#else
#define INITLINEWIDTHLABEL 2.0 
#define INITLINEWIDTHTREE 2		// When using gumby lines instead of cylinders
#define INITLINEWIDTHARC 6.0
#define INITLINEWIDTHARCNAME 2.0
#define LINEWIDTHCIRCLESCALE 1.0		// for the log species number scale 
#endif


/******************** STUFF JUST USED IN CIRCLE VIEW WITH BOXES, ANNOTATIONS, ETC ***************************/

#define DISPLAY_CUTOFF 0.1505		// fractional display values less than this will be drawn white; greater black, if CHAR is on...
//#define DISPLAY_CUTOFF 10000		// fractional display values less than this will be drawn white; greater black, if CHAR is on...
// ...=1.0/5.45126, or 0.27517=1.5/5.45126 median
// ...=0.1484, or 0.0990 for mean...
// ...=0.1505, or 1.5/9.9658 for mean_cfi or ordinal data
#define LN2(a) (log(a)/0.69314718056)
#define MAX_DISPLAY 50
#define NAME_RING_RADIUS 6000
#define CLAMP_CONSTANT	1	// Multiply all brightness values for CHAR boxes by this number, effectively clamping down on high vals
#define BOX_MAX_LENGTH  0.25		// Fraction of gRadiusOfOriginalCircle to make length of boxes
//#define	LN_BOX_MAX_VALUE 7.3132	// Ln Max value of the number stored in a->number2, in case we have to scale it here rather than there!
#define	LN_BOX_MAX_VALUE 8.5279	// Ln Max value of the number stored in a->number2, in case we have to scale it here rather than there!  VALUE FOR 'ORDERS' data
//#define	LN_BOX_MAX_VALUE 7.3492 // Ln Max value of the number stored in a->number2, in case we have to scale it here rather than there! VALUE FOR 'RANKFREE' data

/**********************************************************************************************************/

// ********* Initial tree rotation

float theta[3] ; // rotation matrix for the scene 

// ....This is the initial rotation of the scene

//float theta0[3] ={0.0,0.0,0.0}; // initial rotation good for circle (down z-axis)
float theta0[3] ={270.0,0.0,0}; // initial rotation good for 3D
float theta[3] ; // rotation matrix for the scene 




// ....This is the initial rotation of a leaf object
float objTheta[3] ={0.0,0.0,0.0}; // initial rotation good for circle (down z-axis)



// *******************************

int gNumExpandedFanlets, gDeclutter=1, gSuppressFanTrees=0;
GLfloat gwhite[4]={0.7,0.7,0.7,1.0};
float myColor[3];	// for RGB heat map
long gNumTaxa;
float gBoxWidth;
float gBoxLength;
int gCountLeaves; // used for displaying only every so many leaf labels
int gEveryLeaf;
float zStep,xStep,yStep,thetaStep,theta0step;
int animSteps, animStepsFanlet;
nodeArray gNodeArray;

unsigned int gTexBoxSize=INITGTEXBOXSIZE;

float gLineWidthLabel=INITLINEWIDTHLABEL;
float gLineWidthArc=INITLINEWIDTHARC;
float gLineWidthArcName=INITLINEWIDTHARCNAME;
float gLineWidthTree=INITLINEWIDTHTREE;

float gAnimateScaler = 0.0;
int   gAnimateDirection = +1;
GLubyte * gDiagBitmap;
OglTree gCurrOglTree, gCurrOglSubtree=NULL, gCurrFrontSubtree=NULL;    
float gLabelAttenuate=15;		// this controls how quickly labels fade away with distance from viewer
float gTreeAttenuate=15;		// this controls how quickly branches fade away with distance from viewer
static GLuint texName;
static GLuint gBarkTexName;
int iheight, iwidth;
ppm_image ppm;

int gLeafRotate=0;
int gSpinTree=0;
GLfloat arc_color[4];
float gArcOffset;	// initial factor by which arcs are drawn displaced outside of circumference...
float gTermOffset;	
float gIntOffset;
float gArcLabelOffset;
int gLabelFace=INIT_GLABELFACE;
int moveZ,moveX;


int windowWidth, windowHeight;
int mouseStartX=0, mouseStartY=0;
float mouseFirst=1;
int gIxCol;
int gCorners=G_CORNERS;
int gSuppressTerminals=INIT_GSUPPRESSLABELS;



int sphereMode=1; // 0 = no spheres at joints; 1 = spheres
int axis=0;
int fullScreen=0; // 0 = not; 1 = full screen
int mouseMode=1; // 0 = rotate mode; 1= translate mode
int freezeMode=0; // 0 = screen not frozen, mouse used as input; 1=screen frozen, can use mouse for OS stuff like 'grab'
float baseLabelScale=LABEL_SCALE;
float multLabelScale=1.0;
float labelScale;
float intLabelScale=LABEL_SCALE*INTLABELFACTOR;
float cylinderRadius=CYL_RADIUS;
static GLfloat spin=0.0;
GLUquadricObj *qobj;


GLfloat ground_material_amb_diffuse[] = {0,.2,0 , 1.0 };

// Static function prototypes
static float nodeToEllipseDistance(OglTree T, node n, float a, float b);

static void drawCactusPad(DrawEnum renderFlag, float a, float b, float opacity, short doAttenuate, GLfloat fanColor[] );
void drawTree(DrawEnum render, OglTree T);

static void drawHighlightSelectedTaxon(OglTree T, node a);
static void drawArc(DrawEnum renderFlag, float x0, float y0, float z0, float r,  float theta1, float theta2, float opacity, short doAttenuate, GLfloat color[] );

static void setupTransformationsForTree(OglTree T) ;

static void setDefaultSceneTreeLayouts(OglTree T);
static void drawArcPetiole(DrawEnum renderFlag, OglTree t);
static void drawCurvyCorner(DrawEnum renderFlag, float r,float opacity, short doAttenuate, GLfloat fanColor[] );
static void drawExtendedFanLine(DrawEnum renderFlag, OglTree T);
static void getTubeRadii(node a, float *upperRadius, float *lowerRadius);
static void drawFanBounding(DrawEnum renderFlag,OglTree T);
static void setColorWithAttenuation(float x, float y, float z, GLfloat color[], float attenuateFactor);
static void drawSubtendingFanletLine(DrawEnum render, OglTree T);
static void drawExtendedFanletLine(DrawEnum renderFlag, OglTree T, float length);

static void printSceneTree(OglTree T);

static void bitmapText(int x, int y, char *s, void *font);
static void drawDashboard(OglTree T);


void descArcLimits(node a, float * thetaMin, float * thetaMax);
static int lod_ok(float cutoffz,float x, float y, float z);
static void drawTickBox(node a);
void initTexByNode(node n);
void readNodeImage(node n);
static int getEveryLeaf(float Z,long numTaxa,int maxDisplay);
static float getLabelScale(float mult, float Z, long numtaxa);
static int initializeGLUT(int argc, char** argv);
static void drawCylinderPrecomputed(node a, float radius1,float radius2);
static void initTexBark(void);
static void init(void) ;
static void plane_init(OglTree t);
static void reshape (int w, int h);
static void display(void);
static void drawLabelBox(int labelLen);
static void tileDisplay(void);
static void drawBox(node a);
static void drawText(float angle,float distance,char *s,char justify,float scale,GLfloat *color,float lineWidth);
static void handleSparseLabels(void);
static void drawGroundPlane(void);
static void drawCircleScales(void);
static void descArcLimitsGuts(node a, float *mintTheta, float *maxTheta);
static void drawLabel(node a, enum LayoutChoice layout);
static void animate(double x1, double y1, double z1, double x2, double y2, double z2,int steps);
node curNode;
static void movePlane(float distance);
void dir_update(void);



/****************************************************************/
/**** Entry Point to OpenGL Code ********************************/






int tree_openGL(int argc, char** argv, OglTree theOglTree)
{
    extern int gImages;
    extern node gTreeRoot;
    extern enum TerminalsChoice gTerminalsChoice;
    const GLubyte * infoStr;
    
    gTreeRoot = theOglTree->root;
	gCurrOglTree = theOglTree;
	gCurrOglSubtree = NULL; // none yet selected
    if (gImages)
        preOrderVoid(gTreeRoot,readNodeImage); // traverse the tree, getting images at every node with function 'readNodeImage'

	initLeafMenu(theOglTree); // build a smart hierarchical menu of leaf labels to allow search; NB!! do before cascadeMenu is setup in initializeGlut

    initializeGLUT(argc,argv);
//glEnable(GL_MULTISAMPLE); nope; borks the branch line antialiasing
    plane_init(theOglTree);
    
	theta[0]=theta0[0]; //scene init rotations
	theta[1]=theta0[1];
	theta[2]=theta0[2];
    
   
    init ();
	nurbsInit();

	infoStr = glGetString(GL_VENDOR);
	printf("OpenGL Vendor = %s\n",infoStr);
	
	infoStr = glGetString(GL_RENDERER);
	printf("OpenGL renderer = %s\n",infoStr);
	
	infoStr = glGetString(GL_VERSION);
	printf("OpenGL version = %s\n",infoStr);
	
	//infoStr = glGetString(GL_EXTENSIONS);
	//printf("OpenGL extensions = %s\n",infoStr); this is a long string...
	
	

    glutMainLoop();
    return 0;
}





/****************************************************************/
/****		Initialization routines  ****************************/



static int initializeGLUT(int argc, char** argv)
{
    extern enum TerminalsChoice gTerminalsChoice;
	GLint bufs, samples;
    glutInit(&argc, argv);
    
    if (TILES)
		{
			glutInitMode (GLUT_RGB | GLUT_DEPTH);
			glutInitWindowPosition(0, 0);
			glutInitWindowSize( TILE_WIDTH, TILE_HEIGHT );
		}
    else
		{
			glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE ); // seems to ADD aliasing to lines...sheesh


			//glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH  );
			glutInitWindowSize (INIT_WINDOW_WIDTH,INIT_WINDOW_HEIGHT); 
			glutInitWindowPosition (INIT_WINDOW_POS_X,INIT_WINDOW_POS_Y);
		}
    glutCreateWindow (argv[0]);


			glGetIntegerv(GL_SAMPLE_BUFFERS,&bufs);
			glGetIntegerv(GL_SAMPLES, &samples);
			printf("Multisampling enabled with %i sample buffers and %i samples.\n",bufs,samples);

    
    if (TILES)
        glutDisplayFunc(tileDisplay);
    else
        glutDisplayFunc(display); 
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    glutMotionFunc(mouseMotion);
    //glutMouseFunc(mouse);
    glutMouseFunc(mouse3);
    glutIdleFunc(plane_update);
    
    
#if !WALL
    if (gTerminalsChoice==internals) // for default and -a options we use cascading terminal taxon menus
        setupMenu();
    else					// for -g and -m options, we use internal clade name menus
        setupCascadeMenu();
#endif

   return 0;
}




/*  **************************** MISC INITIALIZATION ROUTINE *****************************
 *
 *  Initialize material property, light source, lighting model,
 *  and depth buffer.
 *  Also initialize tree position and plane position                                */

static void init(void) 
{
	extern float labelScale;
	extern enum ImageType gImageType;
	extern int gImages;
	long numTaxa;
	extern node gTreeRoot;
	extern double gRadiusOfOriginalCircle;
	qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj,GLU_FILL);
	gluQuadricNormals(qobj,GLU_SMOOTH);
    //	gluQuadricTexture(qobj,GL_TRUE);
    
    font_init_tex();
	
	// Nurbs object initializations
    
    gr1=0.5;gz1=0.8;  gr2=0.8; gz2=0.5;  //control point initial values
    

// Interestingly, if we enable multisampling in glut window, then GL turns it on right away without us enabling it. Don't want that
// because it mucks with line antialiasing for example in odd ways, so turn it off, then use it (for now) only in font routine

	glDisable(GL_MULTISAMPLE);
   
    
    // lighting issues for the whole scene
    
	//GLfloat ambient[]={1.0,1.0,1.0,1.0}; // too bright, causes too much washout
	GLfloat ambient[]={.75,.75,.75,1.0}; // this is currently constant across the whole program!
	
   	GLfloat mat_specular[] = { 1.0,1.0,1.0, 1.0 }; // overridden later as with next 2 lines...?
   	GLfloat mat_specular_dull[] = { 0.1,0.1,0.1, 1.0 };
   	GLfloat mat_shininess[] = { 60.0 }; // larger numbers make scene "duller" and seem better for this app
   	GLfloat light_position[] = { 0.0, 0.0, 1.0, 0.0 };

  	glClearColor (bgColor[0], bgColor[1],bgColor[2],bgColor[3]); //effectively the sky when there is a ground plane
	glShadeModel (GL_SMOOTH);
   	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);
   	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_dull);
   	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess); // KEEP!
   	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
   	glEnable(GL_DEPTH_TEST);  // RE-ENABLE TO GO BACK TO OLD CODE...

	//FYI, I only need to enable lighting in drawing cylinders, in drawPrecomputed...()
    
    
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_SRC_ALPHA_SATURATE,GL_ONE); //recommended for antialiasing but hoses other stuff, so leave it as above line for now...
    
	//glEnable(GL_CULL_FACE); // hidden consequence! This hides the back face, which means, if there is a gap in the front of a cylinder join, we can now see through it...Otherwise works fine.

    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST); // Antialiasing is now working! Had to make sure blending was
											// enabled for all cases of branches..
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
#if 0
	initTexBark();
#endif
	if (gImages && gImageType == PPM )
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT,1);
			preOrderVoid(gTreeRoot,initTexByNode); // traverse the tree, initializing textures anywhere there is a label and hence an image
		}
    
	gDiagBitmap = initBitmapDiagLine(1);
	
    // misc...
    
	gArcOffset = gRadiusOfOriginalCircle*ARCOFFSETFACTOR;
	gTermOffset = gRadiusOfOriginalCircle*TERMOFFSETFACTOR;
	gArcLabelOffset = gArcOffset+ gRadiusOfOriginalCircle*ARCLABELOFFSETFACTOR;
    
	numTaxa=numdesc(gTreeRoot);
	gNumTaxa=numTaxa;
	//gBoxWidth=gRadiusOfOriginalCircle*TWO_PI/gNumTaxa/2; // OK, half the box width...
	//gBoxLength=gRadiusOfOriginalCircle*BOX_MAX_LENGTH; // 
    gBoxWidth=BOXW;
	
	if (SPARSE_LABELS)
		labelScale = getLabelScale(multLabelScale,gP->Z,numTaxa);
	else
		labelScale=LABEL_SCALE*multLabelScale;
	gEveryLeaf = getEveryLeaf(gP->Z,numTaxa,MAX_DISPLAY);
    

	gNumExpandedFanlets = 0; // keeps track of how many user expanded fanlets are now visible

}
// ***********************************
//

static void plane_init(OglTree t) 

/* think of this as the position of the eye or viewpoint; set up initially at +Z at a distance based
 on the tree's radius and height, and half way up the tree's height*/

{
    gP=(plane)malloc(sizeof (struct planeStruct));
	gP->X=0.0;
    gP->Y= 0.0;  
	gP->Z=INIT_Z_FACTOR*MAX(t->height,t->radius); 

    gP->Roll=0.0;
	gP->Pitch=0.0;
	gP->Pitch0=0.0;
	gP->Heading=0.0;
	gP->Heading0=0.0;
    
    
	gP->angular_speed=2.5;
	gP->step =1.0;
	gP->speed=0.0;	
	dir_update();
}
//




int gRenderBin,gNumRenderBins;

/****************************************************************/
/******************* OGL DISPLAY ROUTINE ************************/

static void display(void)
{

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

#if 0
// ..rotations of camera/plane here
	//glRotatef(-gP->Roll,0.0,0.0,1.0);
	glRotatef(-gP->Pitch,1.0,0.0,0.0);
	//glRotatef(+gP->Heading,0.0,1.0,0.0);
	glRotatef(+gP->Heading,0.0,0.0,1.0);
#endif

// ..translate if necessary and then rotate and only then draw the scene
	glTranslatef(-gP->X,-gP->Y,-gP->Z);	// shift mouse controlled translations of camera (tree)

	glRotatef(theta[0],1.0,0.0,0.0);	// mouse controlled rotations around the tree's center...
	glRotatef(theta[1],0.0,1.0,0.0);
	glRotatef(theta[2],0.0,0.0,1.0);

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

// Construct the scene


	/*
		Calculate a cumulative modelview matrix at each node in the scene graph, which will save calculations when looping through
		multiple traversals (e.g., different render bins) or displaying only subtrees of the scene graph. This is not quite like
		what is done typically (e.g., Angel's book), where each edge involves a multiplication of the current object's mv matrix with
		the parent's one. I want to isolate it so for any given subtree I immediately know where it came from in mv space.
	*/

	gNumRenderBins = 3; // NB! At the moment, I'm not using #1, so I could go back and economize

	//printSceneTree(gCurrOglTree);

	setupTransformations(gCurrOglTree, MAX_SCENE_TREE_DEPTH); // depth in the scene tree for which this is done
	// I can't move this up to init routines, because of dependence on scene rotations above

	// Use render bins to traverse scene graph several times to control order of object display for blending purposes

	// When there are any expanded fanlets, let's declutter by turning off display of nearby fans
	// except in the subtrees where there is fanlet expansion
	if (gDeclutter && gNumExpandedFanlets>0)
		gSuppressFanTrees = 1;
	else
		gSuppressFanTrees = 0;
		
	for (gRenderBin=1; gRenderBin<=gNumRenderBins; gRenderBin++)
		drawSceneTree(RENDERING, gCurrOglTree, MAX_SCENE_TREE_DEPTH);


	// OK, here's my solution to occlusion of one fan by a nearby fan; redraw it at end of scene traversal 
	// with depth buffering off. Also redraw from the oldest circle to include all descendants.
	// Leads to some occasional wierd effects, mainly I need to turn this off once the viewer has re-rotated the scene (not done yet)

	gSuppressFanTrees = 0;
	OglTree currRootCircle=NULL;

	if (gCurrOglSubtree)
		{
		//if (gCurrOglSubtree->animating) // CAREFUL, the globals get in the way during animation
		//	currRootCircle = gCurrOglSubtree;
		//else 
			currRootCircle = findRootCircle(gCurrOglSubtree); // shouldn't redo this every time
		}

	if (currRootCircle)    
		{
		glDepthFunc(GL_ALWAYS);
		for (gRenderBin=1; gRenderBin<=gNumRenderBins; gRenderBin++) // to control order of blending...
			drawSceneTree(RENDERING, currRootCircle, MAX_SCENE_TREE_DEPTH);
		glDepthFunc(GL_LESS);
		}

	// Finally, bring to front any fan the user has single clicked.
	if (gCurrFrontSubtree)
		{
		//glDepthFunc(GL_ALWAYS); // watch out for this fucker
		for (gRenderBin=1; gRenderBin<=gNumRenderBins; gRenderBin++)
			drawSceneTree(RENDERING, gCurrFrontSubtree, 1);
		glDepthFunc(GL_LESS);
		}


	// displays information at bottom of screen
	drawDashboard(gCurrOglTree); 
	
	glFlush();
	glutSwapBuffers(); 



}

/****************************************************************/
static void drawDashboard(OglTree T)
	{
	char s[100];
	int i;
	extern GLfloat white[];
	glColor4fv(white);
	sprintf(s,"%li leaves   %li subclades (%li visible)",T->nLeaves, T->numSubclades, T->numChildren);
	bitmapText(+10,+5,s,GLUT_BITMAP_HELVETICA_12);
	
	sprintf(s,"Eye = (%5.1f, %5.1f, %5.1f)", gP->X, gP->Y, gP->Z);
	bitmapText(+300,+5,s,GLUT_BITMAP_HELVETICA_12);
	
	sprintf(s,"Angle = (%5.1f, %5.1f)", theta[0], theta[2]);
	bitmapText(+450,+5,s,GLUT_BITMAP_HELVETICA_12);
	
	if (gCurrOglSubtree)
		{
		sprintf(s,"Subclade (%s) %li leaves",gCurrOglSubtree->root->label, gCurrOglSubtree->nLeaves);
		bitmapText(650,+5,s,GLUT_BITMAP_HELVETICA_12);
		}
	
	}
	
static void bitmapText(int x, int y, char *s, void *font)
	{
	int i;
	glWindowPos3i(x, y, 0);
	
	for (i=0;i<strlen(s);i++)
		glutBitmapCharacter(font, s[i]); // keep this font size


	//render_text(s, x,y, 0.1,0.1);
	//printf ("String %s (length = %f)\n",s,myFTStringLength(s));
	}

/**************************************************/
/**************************************************/

/*

Let's store a transformation matrix, mv_matrix, for each subtree in the scene graph, which has the transformations needed to get
to the right place just PRIOR to drawing this current subtree.

I'm departing from the model of scene graph transformations in Angel's book. I find it presently more useful to store the cumulative transformation
up to this point, rather than the isolated transformation for each node in the graph. That way, I can render a subtree instantly, without traversing its ancestors, as when I need to re-render a picked subtree.

*/


void setupTransformations(OglTree T, int maxDepth) 
	// setup the scene tree only to a depth of this, where 1 is the root level, 2 its children, etc.
	{
	OglTree child;
	if (maxDepth == 0) return;
	if (T->anc)
		if (T->layout == fanlet && T->anc->layout == fanlet) return; // we don't descend to fanlets of fanlets (they are "invisible")
	setupTransformationsForTree(T);
	if (maxDepth - 1 == 0 ) return; 
	child = T->firstdesc;
	SIBLOOP(child)
		{
		glPushMatrix();
		setupTransformations(child,maxDepth-1);
		glPopMatrix();
		}

	}


static void setupTransformationsForTree(OglTree T) 
	// The transformations to get to right place just prior to drawing this node (subtree).
	// For fanlets and fans descended from fans, this is the root of the fan(let) but it does not yet include the subtending line extender
	// or the like; this has to be added, along with any subsequent rotations in the actual drawing routine
	
	// Some nasty shit here because when we transition from a 3-d solid cone to a 2-d circle we need to to some coordinate rotations.
	// The former has the the tree going up the z axis (now top of screen). The latter has a tree extending leaf-ward to the right of screen.
	// The theta angle on a node in the 2-d tree is the counterclockwise rotation from the x-axis around the z axis. Once the tree is rotated,
	// the x-axis is the "up" screen direction for these trees, and theta angles are counterclockwise from the vertical.  
	{
	area A;

	//glGetFloatv(GL_MODELVIEW_MATRIX, T->mv_matrix_cumul);


	//glLoadIdentity();


	if (!isRoot(T))
		{
		A=(area)T->parentNode->data;
		glTranslatef(A->x_center,A->y_center,A->z);	
		}


	// Billboard rotation unless the ancestor transformation already includes it!
	if (T->faceStyle == TREE_FACES_VIEWER && !isRoot(T) && T->anc->faceStyle != TREE_FACES_VIEWER)
		{
		glRotatef(-theta[2],0.0,0.0,1.0); // ...derotates the tree to face the viewer. Note that the order of this and the next matters; have to reverse the order of the viewing rotation in display()
		glRotatef(-theta[0],1.0,0.0,0.0); //  
		}

	switch (T->layout)
		{
		case hemisphere:
		case solid_cone:
			if (!isRoot(T))
				glTranslatef(0,0,+T->height/2);

			glGetFloatv(GL_MODELVIEW_MATRIX, T->mv_matrix);

			break;
		case circle: // Take care of the rotations needed here...
			if (T->anc && T->anc->layout == circle) // when we have a circle descended from a circle...need to rotate transform
				{
				glRotatef(+toDegs(A->theta_center ),0.0,0.0,1.0); // ...rotate to have the tree's x-axis parallel to the parent tree's branch
#if CACTUS
                glTranslatef(ellipseRadius(A->theta_center,T->anc->outerRadius*ECC_FACTOR,T->anc->outerRadius)-ellipseRadius(A->theta_center,T->anc->radius*ECC_FACTOR,T->anc->radius),0, 0); // and move to outside border of parent's fan.
                glTranslatef(T->outerRadius*ECC_FACTOR,0,0);
#else
				glTranslatef(T->anc->outerRadius-T->anc->radius,0, 0); // and move to outside border of parent's fan.
				glTranslatef(T->petioleLength +  T->leafBaseLength, 0,0); // and extend this distance beyond (make this a variable)
#endif
               
                }
			if (T->anc && T->anc->branchAttributes->branchStyle==tube) // if the parent is a tube, better offset by radius
				glTranslatef(0,T->parentNode->radius,0);
			if (T->anc && T->anc->layout == solid_cone) // when we have a circle descended from a the solid cone...need to rotate transform
				{
				glRotatef(+90,0.0,0.0,1.0); // 
#if CACTUS
				glTranslatef(T->outerRadius*ECC_FACTOR,0,0);  // here I translate but when circle desc from circle I rely on petiole only
#else
				glTranslatef(T->leafBaseLength,0,0);  // here I translate but when circle desc from circle I rely on petiole only
#endif
				}
			glGetFloatv(GL_MODELVIEW_MATRIX, T->mv_matrix);

			break;

		case fanlet: // consists of the fanlet and its subtending line...Always descended from a circle layout
			glRotatef(+toDegs(A->theta_center ),0.0,0.0,1.0); // ...rotate to have the fan's axis parallel to the parent tree's line from root to terminal taxon...
			glGetFloatv(GL_MODELVIEW_MATRIX, T->mv_matrix);

			break;
		
		}

	}

/**************************************************/
/**************************************************/


void drawSceneTree(DrawEnum renderFlag, OglTree T, int maxDepth) 

// A big phylogeny is rendered as a tree of subtrees, all of which may be visualized with different styles.
// Our scene graph consists of all these trees, arranged in a (scene) tree of phylogenetic trees.
// Thus we render the whole scene by recursively rendering all these phylogenetic trees in whatever style is called for.
// BUT render the scene tree only to a depth of maxDepth, where 1 is the root level, 2 its children, etc.

	{
	OglTree child;
	if (maxDepth == 0) return;

	if (T->anc)
		if (T->layout == fanlet && T->anc->layout == fanlet) return; // we don't descend to fanlets of fanlets (they are "invisible")
	drawTree(renderFlag, T);

	if (maxDepth - 1 == 0 ) return; 
	child = T->firstdesc;
	SIBLOOP(child)
		{
		glPushMatrix();
		drawSceneTree(renderFlag, child,maxDepth-1);
		glPopMatrix();
		}

	}

/**************************************************/

#define FAN_SIZE CIRCLE_FRACTION // get from layout.h 
//#define FAN_OPACITY 1.0
#define FAN_OPACITY 0.55


void drawTree(DrawEnum renderFlag, OglTree T) 

// Draws one of the subtrees in the scene graph.
// Render bounding box for tree, label for tree, and finally call the phylogenetic tree rendering node by node; 
// 'render' flag decides whether to use this for actual rendering to screen or to back buffer for picking
	{
	area A;
	float width=2*T->radius;
	float start, end, biggerRadius;
	if (!isRoot(T))
		{
		A=(area)T->parentNode->data;
		}


	glLoadMatrixf(T->mv_matrix); // puts us in the precomputed correct position to start drawing this (sub)tree

// Drawing order is important here for proper layering of rendering for blending: bounding shape, tree, and label
// Still have problem of labels of subtrees though...

	switch (T->layout)
		{
		case hemisphere:
		case solid_cone:
			if (gRenderBin==1) 
				{
				if (renderFlag == PICKING)
					{
					if (!isRoot(T))
						{
						//colorAndRegisterObject(TREE, T,T->parentNode, -T->outerRadius/2);
						colorAndRegisterObject(FAN, T,NULL, T->outerRadius);
						drawTranslucentBox3d(renderFlag, width,width,T->height,0.2);
						}
					// just ignore the root bounding box for picking
					}
				if (renderFlag == RENDERING)
					drawTranslucentBox3d(renderFlag, width,width,T->height,0.2);
				}

			if (gRenderBin==1 || gRenderBin==2 || gRenderBin==3) drawNode(renderFlag, T,T->root); // will register within function
			//if (gRenderBin==1) 
			//	if (T->root->label)
			//			drawLabelObject(T, T->root, T->internalLabelAttributes,width/2,width/2,+T->height/2);  // FIX BIGGER RADIUS ETC
			break;
		case circle:
			if (gRenderBin==1  /*&& T->nLeaves >= 10*/) 
				{
				// if parent is also a fan, we need to add a subtending edge (or curve), move the fan out, and maybe rotate before drawing it
				if (T->anc && T->anc->layout == circle) // HACK for when we are exploding a fanlet
					{
						drawExtendedFanLine(renderFlag, T);
//drawArcPetiole(renderFlag,T);
//glRotatef(+90,0,0,1);
					}
				if (renderFlag == PICKING) 
					//colorAndRegisterObject(TREE, T,T->parentNode, -T->outerRadius/2);
					colorAndRegisterObject(FAN, T,NULL, T->outerRadius);
				// make sure this registration is after previous transforms so we pick in the right place 
				// (note, if picking fanlet, the yoffset will be ignored in the colorAndReg function call)
				//glTranslatef(T->outerRadius/2.,0,0);
                    drawFanBounding(renderFlag,T);
				}
			if (gRenderBin==1 || gRenderBin==2 || gRenderBin==3) 
				{
				if (T->anc && T->anc->layout == circle) // HACK for when we are exploding a fanlet
					{
					//glRotatef(-90,0,0,1);
					}

				if (!gSuppressFanTrees)
					drawNode(renderFlag, T,T->root);
				}

			// the label on the fan...
#if CACTUS
			if (gRenderBin==2 /* && T->nLeaves >= 10*/) drawLabelObject(T, T->root, T->internalLabelAttributes,+T->outerRadius*ECC_FACTOR,0, 0 ); 
#else
			if (gRenderBin==2 /* && T->nLeaves >= 10*/) drawLabelObject(T, T->root, T->internalLabelAttributes,+T->outerRadius,0, 0 ); 
#endif
			break;

		case fanlet: // consists of the fanlet and its subtending line...
			if (!T->anc) break; // I draw some stuff using parms from the fanlet's ancestor, so it cannot be the root of a display...
			if (gRenderBin==1) 
				{
			//	if (!gSuppressFanTrees)
#if !CACTUS
				if (!gSuppressFanTrees)
					drawSubtendingFanletLine(renderFlag, T); // blending order ok here, because parent circle fan always rendered before child fanlet and line
				glTranslatef(T->anc->outerRadius-T->anc->radius,0, 0); // and move to outside border of parent's fan.
#else
                glTranslatef(ellipseRadius(A->theta_center,T->anc->outerRadius*ECC_FACTOR,T->anc->outerRadius)-ellipseRadius(A->theta_center,T->anc->radius*ECC_FACTOR,T->anc->radius),0, 0); // and move to outside border of parent's fan.
#endif
				if (renderFlag == PICKING) 
					//colorAndRegisterObject(TREE, T,T->parentNode, 0);
					colorAndRegisterObject(FAN, T,NULL, 0);
					
					
				if (T->animating) // this animates the extended line during fanlet explosion
					{
#if CACTUS

					glTranslatef(gAnimateScaler*T->outerRadius*ECC_FACTOR, 0,0);
#else
					glTranslatef(gAnimateScaler*T->petioleLength, 0,0);

#endif
					}
				drawFanBounding(renderFlag,T);
				}

			// for the fanlet, we do not call drawNode

			break;
		
		}


	}
/**************************************************/

void drawNode(DrawEnum renderFlag, OglTree currOglTree, node a)

/*
	Draws all the stuff associated with this node:
		1. subtending branch and possibly connector-sphere
		2. if corners mode on a circle layout, draw the arc centered on this node
		3. label
		4. compact node stuff
		5. images
  
Positions the coord system at the node's ancestor. This is important for drawLabel to recognize!

*/

// Notice that this assigns z value to node's area structure, which means those areas must be created first!
	{

	extern node gCurrPickedNode;
	extern enum ImageType gImageType;
	OglTree oglSubtree;
	extern Hash subclades;
	extern int gCountLeaves, gEveryLeaf, gImages;
	extern GLfloat arc_color[];
	extern GLfloat black[];
	extern double gRadiusOfOriginalCircle;
	extern enum LayoutChoice gLayoutChoice;
	extern enum TerminalsChoice gTerminalsChoice;
	extern int genusMode;
	extern int gIxCol;
	float colx,color;
	int numColIx, numColIx3;
	int i,j,k,count;
	node child;
	node nodeR, nodeL,n;
	area AR, AL,nA;
	double ntheta,minTheta,maxTheta,thetaR, thetaL, rR, rL, zR, zL;
	float thetaMin, thetaMax,offset,thetaName;
	area A,Aanc;
	float cyl_radius,cyl_radius_anc, cone_length=5.0;
	float x0,y0,z0,x1,y1,z1,zr,cross_prodx, cross_prody, cross_prodz;
	char justify;



// Do some initializations



	// ......Set up the cylinder radius....

	getTubeRadii(a,&cyl_radius, &cyl_radius_anc); // now get these from stored node field 'radius'

	A=(area)(a->data);

// Draw the branch itself

	if (gRenderBin==1)
		{
		if (renderFlag == RENDERING) // don't do anything if picking
			{
			// Handle frustum culling

			ExtractFrustum(); // NB! I'll have to set this up again if/when I do more transforms below!!! This WORKS!

			if (BranchInFrustum(A->x_anc, A->y_anc, A->z_anc, A->x_center, A->y_center, A->z)) // frustum culling
				drawBranchObject(a,currOglTree->branchAttributes, cyl_radius_anc,cyl_radius);
			// THERE'S A VISUAL ARTIFACT THAT ARISES BECAUSE WE ARE CULLING BASED ONLY ON ENDPOINTS; SEE FUNCTION

			// For corner layout drawing, draw the arc passing thru this node; precomputed ancestors will have been setup right
			if (currOglTree->branchAttributes->corners && currOglTree->layout != hemisphere && currOglTree->layout != solid_cone)
					drawCornersArcBranch(a, currOglTree, cyl_radius);
			}
		}
// Handle this node if it is a compact node...especially, return without recursing further in

	if (a->isCompactNode)
		{
		if (gRenderBin==1)
				if (renderFlag == RENDERING) // don't do anything if picking
					{
					glPushMatrix();
					glTranslatef(A->x_center,A->y_center,A->z);	

					if (currOglTree->branchAttributes->branchStyle==tube)  // terminal sphere in context of subtree fan, etc.
							{
							glEnable(GL_LIGHTING);
							glEnable(GL_LIGHT0);
							gluSphere(qobj,cyl_radius,15,15);
							glDisable(GL_LIGHTING);
							}
					if (currOglTree->displayInternalLabels)
						drawLabelObject(currOglTree, a, currOglTree->terminalLabelAttributes,0,0,0);		
					glPopMatrix(); 
					}





#if 1
			if (gRenderBin==2 || gRenderBin==3)
					{
					if (nodeHasImages(a))
							// write picture and then handle label specially within the drawPPM routine
						{
						glPushMatrix();
						glTranslatef(A->x_center,A->y_center,A->z);	
						glTranslatef(0,0,cyl_radius);
						if (gRenderBin==2) // make sure to register this only once despite looping through on two render bins
							if (renderFlag == PICKING) 
								//colorAndRegisterObject(IMAGE, currOglTree,a, -BOXW/2); 
								colorAndRegisterObject(IMAGE, currOglTree,a, BOXW); 
						if (gImageType == PPM)
							drawPPM(renderFlag,currOglTree, a, gBoxWidth);
						if (gImageType == STL || gImageType == STLt)
							drawSTL(renderFlag,a, currOglTree->layout);
						if (gImageType == XYZ)
							drawXYZ(renderFlag,a, currOglTree->layout);
						glPopMatrix(); 
						}
					}

#endif





		return;
		}

// not a compact node...
	else
		{ 		// get back in the right spot for drawing labels and such;
		glPushMatrix(); 

		// connector sphere at ancestor's node
		if (gRenderBin==1)
				if (renderFlag == RENDERING) // don't do anything if picking
					if (currOglTree->branchAttributes->branchStyle==tube)
							{
							glEnable(GL_LIGHTING);
							glEnable(GL_LIGHT0);
							glTranslatef(A->x_center,A->y_center,A->z);
							gluSphere(qobj,cyl_radius,10,10); // not the problem with gaps in the tube!
							glTranslatef(-A->x_center,-A->y_center,-A->z);
							glDisable(GL_LIGHTING);
							}

		// draw label if present at this node
		glTranslatef(A->x_center,A->y_center,A->z);	
		if (isTip(a))
			{
			if (gRenderBin==2 || gRenderBin==3)
					{
					if (nodeHasImages(a))
							// write picture and then handle label specially within the drawPPM routine
						{
						glTranslatef(0,0,cyl_radius);
						if (gRenderBin==2) // make sure to register this only once despite looping through on two render bins
							if (renderFlag == PICKING) 
								//colorAndRegisterObject(IMAGE, currOglTree,a, -BOXW/2); 
								colorAndRegisterObject(IMAGE, currOglTree,a, BOXW); 
						if (gImageType == PPM)
							drawPPM(renderFlag,currOglTree, a, gBoxWidth);
						if (gImageType == STL || gImageType == STLt)
							drawSTL(renderFlag,a, currOglTree->layout);
						if (gImageType == XYZ)
							drawXYZ(renderFlag,a, currOglTree->layout);
						}
					else // just write label if no image; label is written WITHIN drawPPM routine above...
						if (gRenderBin==2)
										{
										if (renderFlag == RENDERING) // don't do anything if picking
											if (currOglTree->displayTerminalLabels)
												{
												if (currOglTree->branchAttributes->branchStyle==tube) // only in the case of tubes, we need to offset the label
														// to avoid occlusion of the label by the tube itself
													drawLabelObject(currOglTree, a, currOglTree->terminalLabelAttributes,0,0,cyl_radius );		
												else
													drawLabelObject(currOglTree, a, currOglTree->terminalLabelAttributes,0,0,0);		
												if (a==gCurrPickedNode)
													drawHighlightSelectedTaxon(currOglTree, a);
												}
										}
				}		
			}
		glPopMatrix();	
		
//...and recurse		
		child=a->firstdesc;
//		if (!isTip(a) && (!a->label || (a->label && gTerminalsChoice != internals))   )
			SIBLOOP(child)
				drawNode(renderFlag,currOglTree,child);
		return;
		}
	}
void drawHighlightSelectedTaxon(OglTree T, node a) // rather than considering this an object with a transform, etc...just do this
	{
	extern GLfloat red[];
	area A = (area)(a->data);
	if (T->layout == circle)
		{
		float x,y,size;
		float labelLen = glutStrokeLength(T->terminalLabelAttributes->font, a->label)*T->terminalLabelAttributes->scaleSize;

		glRotatef(+toDegs(A->theta_center ),0.0,0.0,1.0); 
		setColorWithAttenuation(0,0,0,red, T->branchAttributes->attenuateFactor );
		x=T->outerRadius-T->radius;
		size = x/5.0;
		glBegin(GL_LINES);
		glVertex3f(labelLen,-0.01,0);
		glVertex3f(x,-0.01,0);
		glEnd();
		glBegin(GL_QUADS);
		glVertex3f(x,-0.01,0);
		glVertex3f(x+size,size/2,0);
		glVertex3f(x+2*size,0,0);
		glVertex3f(x+size,-size/2,0);
		glEnd();
					//glTranslatef(T->anc->outerRadius-T->anc->radius,0, 0); // and move to outside border of parent's fan.
		}

	}
/**************************************************/
/**************************************************/
/**************************************************/



static void getTubeRadii(node a, float *upperRadius, float *lowerRadius)
		// For branch subtending node, upper radius is at the node itself; lower radius is at the ancestor node.
		// This routine sets the radius based on the order of the node.
		{
			float delta;
			*upperRadius = a->radius;
			if (isRoot(a))
				*lowerRadius = a->radius;
			else
				*lowerRadius= a->anc->radius; // branch under root is a cylinder

		}


// ***********************************

static int lod_ok(float cutoffz,float x, float y, float z)

// returns a 1 if the object at obj coords x,y,z, is less than cutoffz distance from eyepoint
// used for level of detail control

{
GLfloat params[4];
GLboolean paramsb[1];
glRasterPos3f(x,y,z);
glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,paramsb);
if (paramsb[0]==GL_FALSE)
	return 0;
else
	{
	glGetFloatv(GL_CURRENT_RASTER_DISTANCE,params);
	if (params[0] < -cutoffz )
		return 0;
	else
		return 1;
	}
}


//
// ***********************************
//


static void drawSubtendingFanletLine(DrawEnum renderFlag, OglTree T)
	{
	//don't do anything about rendering at the moment, but be sure to not write this to back buffer
	extern GLfloat yellow[];
	if (!T->anc) return; // I draw some stuff using parms from the fanlet's ancestor, so it cannot be the root of a display...
	float labelLen = glutStrokeLength(T->anc->terminalLabelAttributes->font, T->parentNode->label)*T->anc->terminalLabelAttributes->scaleSize;

	glLineWidth(T->anc->branchAttributes->lineWidth);
	setColorWithAttenuation(0,0,0,yellow, T->anc->branchAttributes->attenuateFactor );
	glBegin(GL_LINES);
	glVertex3f(labelLen,-0.01,0);
	glVertex3f(T->anc->outerRadius-T->anc->radius,-0.01,0);
	glEnd();
	}


static void drawExtendedFanletLine(DrawEnum renderFlag, OglTree T, float length)
	{
	//don't do anything about rendering at the moment, but be sure to not write this to back buffer
	extern GLfloat yellow[];
	if (!T->anc) return; // I draw some stuff using parms from the fanlet's ancestor, so it cannot be the root of a display...

	glLineWidth(T->anc->branchAttributes->lineWidth);
	setColorWithAttenuation(0,0,0,yellow, T->anc->branchAttributes->attenuateFactor );
	glBegin(GL_LINES);
	glVertex3f(0,-0.01,0);
	glVertex3f(length,-0.01,0);
	glEnd();
	}
static void drawExtendedFanLine(DrawEnum renderFlag, OglTree T)
	{
	//draw a line from parent node coords in the direction of x,y,z
	extern GLfloat yellow[];
	if (!T->anc) return; // I draw some stuff using parms from the fanlet's ancestor, so it cannot be the root of a display...
	float labelLen = glutStrokeLength(T->anc->terminalLabelAttributes->font, T->parentNode->label)*T->anc->terminalLabelAttributes->scaleSize;

	float length = (T->anc->outerRadius-T->anc->radius) - labelLen + T->petioleLength + T->leafBaseLength;


	glLineWidth(T->branchAttributes->lineWidth); 
	float fanAttenuateFactor= 5.0; // decays about 1/3 as fast as fan's branches...
	setColorWithAttenuation(0,0,0,yellow, fanAttenuateFactor );
	glBegin(GL_LINES);
	glVertex3f(-length,-0.01,0);
	glVertex3f(0,-0.01,0);
	glEnd();
	}
	

// Hmm revisit the code above and below; seems like I should use the line segment version of this code...

static void setColorWithAttenuation(float x, float y, float z, GLfloat color[], float attenuateFactor)
	{
	GLfloat params[4];
	GLboolean valid;
	float darkness;
	glRasterPos3f(x,y,z);
	glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,&valid);
	if (valid) // sometimes one end of the branch is off screen, causing eye distance to be undefined...
		{
		glGetFloatv(GL_CURRENT_RASTER_DISTANCE,params);
		darkness = 1 + attenuateFactor*params[0]/1000;
		}
	else
			darkness = 0.0;  // case where both ends are offscreen; OGL should handle this anyway.
		
	if (darkness > 1) darkness = 1;
	if (darkness < 0) darkness = 0;

	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(color[0],color[1],color[2],darkness);
	}



static void drawLabelBox(int labelLen)
			// this makes semi-transparent banners behind the taxon labels
			// Notice that the blending is quite dependent on the order that labels and branches get drawn:
			// If the branch is drawn after the label, it will not be blended--it will be occluded completely;
			// the only soln is to draw all the branches first, but is it too expensive to recurse 
			// the whole tree again?
	{
		float darkness = 0.3;
		//glEnable(GL_BLEND); 
		//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		//glColor4f(0,0,0,darkness);
		glBegin(GL_QUADS);
		glVertex3f(-10,-100,-10);  // these constants come from God
		glVertex3f(-10,60,-10);
		glVertex3f(labelLen+20,60,-10);
		glVertex3f(labelLen+20,-100,-10);
		//glDisable(GL_BLEND);
		glEnd();
	}


void drawTranslucentBox(float width, float height, float z_offset, float opacity)
			// this makes semi-transparent banners behind the taxon labels
			// Notice that the blending is quite dependent on the order that labels and branches get drawn:
			// If the branch is drawn after the label, it will not be blended--it will be occluded completely;
			// the only soln is to draw all the branches first, but is it too expensive to recurse 
			// the whole tree again?
	{
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0,0,0,opacity);
		glBegin(GL_QUADS);
		glVertex3f(0,0,z_offset);  // these constants come from God
		glVertex3f(0,height,z_offset);
		glVertex3f(width,height,z_offset);
		glVertex3f(width,0,z_offset);
		glDisable(GL_BLEND);
		glEnd();
	}
void drawSquare(float width)
	{
	float x,y,z;
	x=y=z=width/2.0;
	glBegin(GL_QUADS);
	glVertex3f(-x,-y,-z); 
	glVertex3f(-x,+y,-z);
	glVertex3f(+x,+y,-z);
	glVertex3f(+x,-y,-z);
	glEnd();
	}

void drawTranslucentBox3d(DrawEnum renderFlag, float x, float y, float z, float opacity)

// version with outline

	{
		GLenum shape;
		if (renderFlag == RENDERING)
			shape = GL_LINE_LOOP;
		if (renderFlag == PICKING)
			shape = GL_QUADS;
			
		if (renderFlag == RENDERING)
			{
			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(0,0,0,opacity);
			glDepthMask(GL_FALSE); // have to do this to prevent hidden line removal of objects behind the translucent floor of the box
			}
		else
			glDisable(GL_BLEND);

		x/=2.0;
		y/=2.0;
		z/=2.0;
// REDO ALL This respecting right hand rule: counterclockwise vertices are the outward face


		glBegin(GL_QUADS);
		glVertex3f(-x,-y,-z); 
		glVertex3f(-x,+y,-z);
		glVertex3f(+x,+y,-z);
		glVertex3f(+x,-y,-z);
		glEnd();
	if (renderFlag == RENDERING)
		glDepthMask(GL_TRUE);

		glBegin(shape);
		glVertex3f(-x,-y,+z); 
		glVertex3f(-x,+y,+z);
		glVertex3f(+x,+y,+z);
		glVertex3f(+x,-y,+z);
		glEnd();

		glBegin(shape);
		glVertex3f(-x,-y,-z); 
		glVertex3f(-x,-y,+z); 
		glVertex3f(-x,+y,+z); 
		glVertex3f(-x,+y,-z); 
		glEnd();

		glBegin(shape);
		glVertex3f(+x,-y,-z); 
		glVertex3f(+x,-y,+z); 
		glVertex3f(+x,+y,+z); 
		glVertex3f(+x,+y,-z); 
		glEnd();

		glBegin(shape);
		glVertex3f(-x,-y,-z); 
		glVertex3f(-x,-y,+z); 
		glVertex3f(+x,-y,+z); 
		glVertex3f(+x,-y,-z); 
		glEnd();

		glBegin(shape);
		glVertex3f(-x,+y,-z); 
		glVertex3f(-x,+y,+z); 
		glVertex3f(+x,+y,+z); 
		glVertex3f(+x,+y,-z); 
		glEnd();

	if (renderFlag == RENDERING)
		glDisable(GL_BLEND);
	}

//
// ***********************************
//

static void drawText(float angle,float distance,char *s, char justify,float scale,GLfloat *color,float lineWidth)
{
// Display text horizontally on screen at radius of distance and angle on the tree given, and correct for viewpoint rotation
	char *p;
	int labelLen;
	glPushMatrix();
	glRotatef(angle,0.0,0.0,1.0); 
	glTranslatef(distance,0.0,0.0);
	glRotatef(-angle-theta[2],0.0,0.0,1.0); 
	glColor4fv(color);
	glLineWidth(lineWidth);
	glScalef(scale,scale,scale); // make the internal labels this size
	replace_underscore(s);		// hack, because I obviously don't have to do this a million times...
	labelLen = glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)s); // func is choosy about its pointer type
	if (justify=='L')
		glTranslatef(-labelLen,0.0,0.0);	//left justify name for use on left side of tree
	for (p = s; *p; p++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	glPopMatrix();
}
//
// ***********************************
//

void descArcLimits(node a, float * thetaMin, float * thetaMax)
// given a node a, finds the min and max theta across ALL descendants of that node;
// useful for drawing an arc at the radius of the circumference to annotate layout with a clade name
	{
	*thetaMin=1000,*thetaMax=-1000;
	descArcLimitsGuts(a,thetaMin,thetaMax);
	return;
	}
//
// ***********************************
//

static void descArcLimitsGuts(node a, float *minTheta, float *maxTheta)
	{
	area nA;
	float ntheta;
	node n;
	if (isTip(a)) return;
	n=a->firstdesc;
	SIBLOOP(n)
		{
		nA=(area)(n->data);
		ntheta=nA->theta_center;
		if (ntheta>*maxTheta) *maxTheta=ntheta;
		if (ntheta<*minTheta) *minTheta=ntheta;	
		descArcLimitsGuts(n,minTheta,maxTheta);
		}
	return;
	}

//
// ***********************************
//

void dir_update(void)
	{
#if 1
	float radius;
	radius = cos (toRads(gP->Pitch));
	gP->Ydir=sin(toRads(gP->Pitch));
	gP->Xdir=sin(toRads(gP->Heading))*radius;
	gP->Zdir=-cos(toRads(gP->Heading))*radius;
#else
	gP->Xdir=0.0;
	gP->Ydir=0.0;
	gP->Zdir=-1.0;
#endif

//printf("%f\t%f\t%f\n",gP->Heading,gP->Xdir,gP->Zdir);
	}
void plane_update(void) // now a misnomer because we will handle spinning the tree here too for the 'O' option
	{
	unsigned long i;
	dir_update();
	movePlane(gP->speed);
	glutPostRedisplay();
	axis=2; //required global for next call
	if (gSpinTree)
		spinDisplay(gSlowSpinAngle);
	}
void movePlane(float distance)
	{
	gP->X += gP->Xdir*distance;
	gP->Y += gP->Ydir*distance;
	gP->Z += gP->Zdir*distance;
	}
void spinDisplay(float angle)
{
long i;
theta[axis]+=angle;
if (theta[axis]>360.0)
	theta[axis]-=360.0;
if (theta[axis]<0.0)
	theta[axis]+=360.0;
glutPostRedisplay();
}





static float getLabelScale(float mult, float Z, long numtaxa)
	{
	extern double gRadiusOfOriginalCircle;
	return LABEL_SCALE *mult*fabs(Z)/gRadiusOfOriginalCircle;
	}
static int getEveryLeaf(float Z,long numTaxa,int maxDisplay)
	{
	extern double gRadiusOfOriginalCircle;
	int every;
	if (numTaxa<maxDisplay) return 1;
	every = 1 + LN2((float)numTaxa/maxDisplay);

	every = floor(   (every-1)* pow(fabs(Z)/gRadiusOfOriginalCircle,0.50)  );
	return pow(2,every);
	}


static void handleSparseLabels(void)
{
	extern float labelScale;
	float max_gEvery;
	float z1=20;
	float z0=2;
	float yy;
	labelScale=getLabelScale(multLabelScale,gP->Z,gNumTaxa);
	gEveryLeaf = 50000/gNumTaxa*fabs(gP->Z)/2;
	max_gEvery=gNumTaxa/MAX_DISPLAY;
	gEveryLeaf = ((max_gEvery-1)/(z1-z0))*(fabs(gP->Z)-z0) + 1;
	yy=fabs(gP->Z)/3;
	gEveryLeaf = floor(log(2)*log( pow(2,floor(4*yy)) )) * floor (pow (2, floor(pow(yy,0.3)))); 
	if (gEveryLeaf < 1.0) gEveryLeaf=1.0;
	if (gNumTaxa < 20) gEveryLeaf=1;
	gEveryLeaf = getEveryLeaf(gP->Z,gNumTaxa,MAX_DISPLAY);
}


#if 0
static void drawGroundPlane(void)
{
   	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ground_material_amb_diffuse);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glColor3f(GROUND_RED,GROUND_GREEN,GROUND_BLUE);
	glBegin(GL_QUADS);
	glVertex3f(-GROUND_PLANE_SIZE,-GROUND_PLANE_SIZE,-ROOT_LENGTH);
	glVertex3f(-GROUND_PLANE_SIZE,GROUND_PLANE_SIZE,-ROOT_LENGTH);
	glVertex3f(+GROUND_PLANE_SIZE,+GROUND_PLANE_SIZE,-ROOT_LENGTH);
	glVertex3f(+GROUND_PLANE_SIZE,-GROUND_PLANE_SIZE,-ROOT_LENGTH);
	glEnd();
}
#endif


int nodeLabelCompar(const void *elem1, const void * elem2)

// sort comparison function: alphabetical sort, skipping over first non-alpha characters in string
// and if the string is ALL nonalpha, it gets sorted to end of list

{
node node1,node2;
char s1[256],s2[256],*S1, *S2;
node1=*(node *)elem1; // ugh, first cast the void pointer for the comparator, then deref to get the node
node2=*(node *)elem2;
strcpy(s1,node1->label);
strcpy(s2,node2->label);
S1=firstalpha(s1);
S2=firstalpha(s2); // move into string until we get to first alphabet char
if (!S1) return 1;
if (!S2) return -1; // if one of the strings has no alpha chars, this will insure it gets sorted last?
strtoupper(S1);
strtoupper(S2);
return strcmp(S1,S2);
}

void strtoupper(char *s)
{
while (*s)
	{
	*s=toupper(*s);
	s++;
	}
}

char * firstalpha(char *s) // returns pointer to first alphabetical character in string, or null if none.
{
while (*s)
	{
	if (isalpha(*s)) return (s);
	++s;
	}
return 0;
}





void replace_underscore(char *p)
	{
	for (;*p;p++) 
		if (*p=='_') *p=' ';
	}


/*
 * Below is some code from 'trdemo2.c' to write a tiled version to file
 *
 * Brian Paul
 * April 1997
 */


/* $Id: trdemo2.c,v 1.5 1999/01/24 17:25:46 brianp Exp $ */



/* Do a demonstration of tiled rendering */
static void tileDisplay(void)
{
   TRcontext *tr;
   GLubyte *buffer;
   GLubyte *tile;
   FILE *f;
   int more;
   int i;
	float aspectRatio;

   printf("Generating %d by %d image file...\n", IMAGE_WIDTH, IMAGE_HEIGHT);


   /* allocate buffer large enough to store one tile */
   tile = malloc(TILE_WIDTH * TILE_HEIGHT * 3 * sizeof(GLubyte));
   if (!tile) {
      printf("Malloc of tile buffer failed!\n");
      return;
   }

   /* allocate buffer to hold a row of tiles */
   buffer = malloc(IMAGE_WIDTH * TILE_HEIGHT * 3 * sizeof(GLubyte));
   if (!buffer) {
      free(tile);
      printf("Malloc of tile row buffer failed!\n");
      return;
   }

   /* Setup.  Each tile is TILE_WIDTH x TILE_HEIGHT pixels. */
   tr = trNew();
   trTileSize(tr, TILE_WIDTH, TILE_HEIGHT, TILE_BORDER);
   trTileBuffer(tr, GL_RGB, GL_UNSIGNED_BYTE, tile);
   trImageSize(tr, IMAGE_WIDTH, IMAGE_HEIGHT);
   trRowOrder(tr, TR_TOP_TO_BOTTOM);

//   trFrustum(tr, -1.0, 1.0, -1.0, 1.0, 6.0, 25.0);
// ...follows is mjs snippet
//   aspectRatio=(float)windowWidth/windowHeight;
//   if (windowWidth <= windowHeight)
//   aspectRatio=(float)TILE_WIDTH/TILE_HEIGHT;
//   if (TILE_WIDTH <= TILE_HEIGHT)
   aspectRatio=(float)IMAGE_WIDTH/IMAGE_HEIGHT;
   if (IMAGE_WIDTH <= IMAGE_HEIGHT)
      trFrustum (tr,-2.0, 2.0, -2.0/aspectRatio, 2.0/aspectRatio, FRONTCLIP,BACKCLIP);
   else
      trFrustum (tr,-2.0*aspectRatio, 2.0*aspectRatio, -2.0, 2.0, FRONTCLIP,BACKCLIP);
// ...end snippet

   /* Prepare ppm output file */
   f = fopen(FILENAME, "w");
   if (!f) {
      printf("Couldn't open image file: %s\n", FILENAME);
      return;
   }
   fprintf(f,"P6\n");
   fprintf(f,"# ppm-file created by %s\n", "trdemo2");
   fprintf(f,"%i %i\n", IMAGE_WIDTH, IMAGE_HEIGHT);
   fprintf(f,"255\n");
   fclose(f);
   f = fopen(FILENAME, "ab");  /* now append binary data */
   if (!f) {
      printf("Couldn't append to image file: %s\n", FILENAME);
      return;
   }

   /*
    * Should set GL_PACK_ALIGNMENT to 1 if the image width is not
    * a multiple of 4, but that seems to cause a bug with some NVIDIA
    * cards/drivers.
    */
   glPixelStorei(GL_PACK_ALIGNMENT, 4);

   /* Draw tiles */
   more = 1;
   while (more) {
      int curColumn;
      trBeginTile(tr);
      curColumn = trGet(tr, TR_CURRENT_COLUMN);
      display();      /* this is the paloverde display function */
      more = trEndTile(tr);

      /* save tile into tile row buffer*/
      {
	 int curTileWidth = trGet(tr, TR_CURRENT_TILE_WIDTH);
	 int bytesPerImageRow = IMAGE_WIDTH*3*sizeof(GLubyte);
	 int bytesPerTileRow = (TILE_WIDTH-2*TILE_BORDER) * 3*sizeof(GLubyte);
	 int xOffset = curColumn * bytesPerTileRow;
	 int bytesPerCurrentTileRow = (curTileWidth-2*TILE_BORDER)*3*sizeof(GLubyte);
	 int i;
	 int curTileHeight = trGet(tr, TR_CURRENT_TILE_HEIGHT);
	 for (i=0;i<curTileHeight;i++) {
	    memcpy(buffer + i*bytesPerImageRow + xOffset, /* Dest */
		   tile + i*bytesPerTileRow,              /* Src */
		   bytesPerCurrentTileRow);               /* Byte count*/
	 }
      }
      
      if (curColumn == trGet(tr, TR_COLUMNS)-1) {
	 /* write this buffered row of tiles to the file */
	 int curTileHeight = trGet(tr, TR_CURRENT_TILE_HEIGHT);
	 int bytesPerImageRow = IMAGE_WIDTH*3*sizeof(GLubyte);
	 int i;
	 GLubyte *rowPtr;
         /* The arithmetic is a bit tricky here because of borders and
          * the up/down flip.  Thanks to Marcel Lancelle for fixing it.
          */
	 for (i=2*TILE_BORDER;i<curTileHeight;i++) {
	    /* Remember, OpenGL images are bottom to top.  Have to reverse. */
	    rowPtr = buffer + (curTileHeight-1-i) * bytesPerImageRow;
	    fwrite(rowPtr, 1, IMAGE_WIDTH*3, f);
	 }
      }

   }
   trDelete(tr);

   fclose(f);
   printf("%s complete.\n", FILENAME);

   free(tile);
   free(buffer);

   exit(0);
}

static void initTexBark(void)
	{
	GLuint texName;
	ppm_image ppm;
	char filename[32];
	sprintf(filename,"images/bark.ppm");	
	ppm = (ppm_image)getPPM(filename);
	if (ppm == NULL)
		printf("Error reading image file %s\n",filename);
	else
		{
		glGenTextures(1,&texName);
		glBindTexture(GL_TEXTURE_2D, texName);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, ppm->width, ppm->height, GL_RGB, GL_UNSIGNED_BYTE, ppm->image);
		gBarkTexName=texName;
		}
	return;
	}



static void setDefaultSceneTreeLayouts(OglTree T)

/* Specialized routine to engineer collapsing of a multilevel expanded fanlet subtree, keeping track of how many are lost.
Need to do this when we double click somewhere not at the leaf of this subtree but deeper */

// NOT WORKING YET...

	{
	OglTree child,sib;
	if (T->layout == circle && T->defaultLayout==fanlet)
		{
		T->layout = T->defaultLayout;
		--gNumExpandedFanlets;
		}
	child = T->firstdesc;
	SIBLOOP(child)
	if (child)
		setDefaultSceneTreeLayouts(child);
	}

int maxSteps;
void fanletAnimation(OglTree t, short direction)
	{
	extern float gTargetX, gTargetY;
	area A = (area)(t->parentNode->data);
	maxSteps = 25;
	float angle = A->theta_center - t->anc->upDirection;
	gTargetX = sin(angle)*(t->anc->outerRadius+t->petioleLength  + t->leafBaseLength);
	gTargetY = cos(angle)*(t->anc->outerRadius+t->petioleLength  + t->leafBaseLength); // sin and cos are backwards from intuition; remember the fans go to right before being rotated by 90 degrees to vertical
	gAnimateDirection=direction;
	animStepsFanlet=0;
	t->animating=1;
	
	if (gAnimateDirection == +1)  // animate in direction of exploding fanlet
		{
		//gAnimateScaler = animStepsFanlet/(float)maxSteps; 
		gNumExpandedFanlets++;
		}
	if (gAnimateDirection == -1)
		{
		setDefaultSceneTreeLayouts(gCurrOglSubtree);  // takes care of case in which we want to collapse a whole subtree of expanded fanlets

		gCurrOglSubtree->layout=fanlet; // yes, need to set this to fanlet first to get ride behavior (only fanlets get imploded/exploded)
		//gAnimateScaler = 1 - animStepsFanlet/(float)maxSteps; 
		}
	
	glutIdleFunc(animateFanlet);
	setupTransformations(t,MAX_SCENE_TREE_DEPTH); // 
	return;
	}


void animateFanlet(void)
{
extern float gTargetX, gTargetY; // defined in picking.c -- move back to here at some point

	if (gAnimateDirection == +1)  // animate in direction of exploding fanlet
		gAnimateScaler = animStepsFanlet/(float)maxSteps;
	if (gAnimateDirection == -1)
		gAnimateScaler = 1 - animStepsFanlet/(float)maxSteps; 
	
	gP->X -= gAnimateDirection * gTargetX/maxSteps; 
	gP->Y += gAnimateDirection * gTargetY/maxSteps; 
	if (animStepsFanlet++==maxSteps)
		{
		gCurrOglSubtree->animating=0;
		if (gAnimateDirection == +1)
			gCurrOglSubtree->layout=circle;
		glutIdleFunc(plane_update); // restore to default idle function
		}	
glutPostRedisplay();
return;
}


void animateTwist(void)
{
extern float zStep,thetaStep,xStep,theta0step;
extern int animSteps;
    int i;
    for (i=0;i<100000;i++);  // not currently the rate limiting step in trees >= 100s of taxa
if (animSteps--==0)
	{
	glutIdleFunc(plane_update); // restore to default idle function
//printf("Final theta = %f\n",theta[2]);
	return;
	}
theta[2]+=thetaStep; // this is adjusting the angle around the z-axis
theta[0]+=theta0step; // this is putting the tree back to its completely vertical original pos
gP->Z+=zStep;
gP->Y+=yStep;//!!! notice cross talk on the variable names because of having rotated the tree to vert
gP->X+=xStep;
glutPostRedisplay();
return;
}

int boxAnimSteps,gDir;
double boxThetaStep;
int arrayIndex;
void animateBoxRotate(void)
{
if (boxAnimSteps-- == 0)
	{
	glutIdleFunc(plane_update); // restore to default idle function
	return;
	}
objTheta[arrayIndex]+=boxThetaStep; // this is adjusting the angle around the z-axis

// TO DO: Fix the accumulating roundoff error if we just hold the arrow key down

glutPostRedisplay();
return;
}




void readNodeImage(node n) 
				// tries to read one or more image files at this node based on the node's name
			   // assumes images are in subdirectory ../images
			   // If any images are found, an array of images is created and stored at n->data2, else a NULL is stored there
	{
	extern enum ImageType gImageType;
	extern int gMaxImagesPerNode;
	int face,hasImages;
	char filename[128],filename_underscore[128], *f;
	ppm_image image;
	ppm_image *image_array; // array of pointers to ppms
	if (n->label)
		{
		switch (gImageType)
			{
			case PPM:
				hasImages=0;
				image_array = (ppm_image*)malloc(gMaxImagesPerNode*sizeof(ppm_image));
				sprintf(filename,"images/PPM/%s.ppm",n->label,face);	
				for (face = 0; face<gMaxImagesPerNode; face++)
					{
					sprintf(filename_underscore,"images/PPM/%s_%i.ppm",n->label,face);	
					if (gMaxImagesPerNode>1)
						{
						image = (ppm_image)getPPM(filename_underscore);
						if (!image)
							image = (ppm_image)getPPM(filename);
						}
						
					else
						image = (ppm_image)getPPM(filename);
					if (image)
						{
						hasImages=1;
						image_array[face]=image; //image;
						printf("Found, read and stored image from %s\n",filename);
						

#if PPM_TESTING
						extern node gPPM_Test_Node;
						gPPM_Test_Node = n;
#endif
						}
					else
						{
						image_array[face]=NULL;
						printf("...%s not found, read and stored image from \n",filename);
						}
					}
				if (hasImages)
					n->data2 = image_array;
				else
					{
					n->data2 = NULL;
					free (image_array);
					}
				break;
			case STL:
				sprintf(filename,"images/STL/%s.stl",n->label);	
				n->data2 = (STL_Array*)getSTL(filename);
				break;
			case STLt:
				sprintf(filename,"images/STLt/%s.STLt",n->label);	
				n->data2 = (STL_Array*)getSTLt(filename);
				break;
			case XYZ:
				sprintf(filename,"images/XYZ/%s.xyz",n->label);	
				//n->data2 = (XYZ_Array*)getXYZ(filename);
				n->data2 = (XYZ_Array*)getXYZ_v2(filename);
				break;
			}
		//if (n->data2 == NULL)
		//	printf("Error reading image file %s\n",filename);
		}
	}
	
	
	/********************* CYLINDERS ETC    ************************/

//
// ***********************************
//

void setup_cylinders_trig(OglTree oglTree, node a)

// do the trig and stuff to pack translate and rotate info into each node for subsequent drawNode calls
// oh, there's probably clever openGL matrix xformations to do this without the trig 
// To draw the cylinder with glut we have to figure out the direction the branch is pointing, and then
// figure out the normal to that, and rotate by the right angle around that normal axis. That's the trig
	{
	enum LayoutChoice layout;
	extern enum TerminalsChoice gTerminalsChoice;
	char *p;
	node child;
	area A,Aanc;
	float cyl_radius;
	float x1,y1,z1,theta,zr;
	layout = oglTree -> layout;
	A=(area)(a->data);
	if (!isRoot(a)) 
		{
		Aanc=(area)(a->anc->data);
		A->x_anc=Aanc->x_center;
		A->y_anc=Aanc->y_center;
		A->z_anc=Aanc->z;
		if (oglTree->branchAttributes->corners && layout != hemisphere && layout != solid_cone)
			{
			A->x_anc= (Aanc->r1)*cos(A->theta_center);
			A->y_anc= (Aanc->r1)*sin(A->theta_center);
			}
		}
	else
		{
		if (!isRoot(oglTree))
		//if (oglTree->parentTree) // if the tree is a subtree don't draw the root branch; hmm need to do this for x-y too
			A->z_anc=A->z;
		else
			A->z_anc=A->z - ROOT_LENGTH;
		A->x_anc=0.0;
		A->y_anc=0.0;
		}
	x1=A->x_center - A->x_anc;
	y1=A->y_center - A->y_anc;
	z1=A->z - A->z_anc;
	A->zr = sqrt(x1*x1+y1*y1+z1*z1);
	A->cross_prodx = -y1;
	A->cross_prody = +x1;
//	A->cross_prodz = 0.0; // obviously because this is always 0, we don't have to store at each node
	A->theta = toDegs( acos (z1/A->zr)) ; // 

	if (x1==0.0 && y1==0.0)
		A->rho=0.0;
	else
		A->rho = toDegs( atan(x1/y1));

//if (a->label && gTerminalsChoice == internals) return;
	if (a ->isCompactNode) return;
	child=a->firstdesc;
	SIBLOOP(child)
		setup_cylinders_trig(oglTree, child);
	return;
	}

//
// ***********************************
static void drawFanBounding(DrawEnum renderFlag,OglTree T)
{
// Animation controlled by gAnimateScaler which is [0,1].
//float gAnimateScaler=1.0;

// Origin of this object is at the root node. The leaf base extends back down -x axis.

extern GLfloat lt_yellow[];
GLfloat dark_green[4]={0,.15,0,1};
float start,end,numturns;
float curRadius, curMinTheta, curMaxTheta;
numturns = 1.0 * T->circleFraction;
float fanletRadius = T->anc->outerRadius/10.;
float fanletMinTheta = -0.2, fanletMaxTheta = +0.2; 
float circleRadius = T->outerRadius, circleMinTheta, circleMaxTheta;
float lfBase = T->leafBaseLength; // this will be the length of the leaf base extending backwards
start = (1-numturns)/2.0;
end = 1-(1-numturns)/2.0;


circleMinTheta = start*TWO_PI-PI;
circleMaxTheta = end*TWO_PI-PI;  // these are angles centered on 0...

if (T->layout == circle)
	{

#if CACTUS
		drawCactusPad(renderFlag, circleRadius*ECC_FACTOR, circleRadius, FAN_OPACITY, T->doAttenuateBoundingShape, dark_green  );
        
#else
        drawFan(renderFlag, 0,0,-0.01, circleRadius,  circleMinTheta, circleMaxTheta, FAN_OPACITY, T->doAttenuateBoundingShape,dark_green );  
		// bunch of crap to draw the curvy petiole -- simplify this later.
		glTranslatef(-lfBase,0,0);
		drawCurvyCorner(renderFlag, lfBase,FAN_OPACITY, T->doAttenuateBoundingShape,dark_green  );
		glScalef(1,-1,1);
		drawCurvyCorner(renderFlag, lfBase, FAN_OPACITY, T->doAttenuateBoundingShape,dark_green  );
		glScalef(1,-1,1);  // slow??
		glTranslatef(+lfBase,0,0);
#endif

	}
if (T->layout == fanlet)
	{
	if (T->animating)
		{
#if CACTUS
		drawCactusPad(renderFlag, circleRadius*ECC_FACTOR*gAnimateScaler, circleRadius*gAnimateScaler, FAN_OPACITY, T->doAttenuateBoundingShape, dark_green  );

#else

		curRadius = fanletRadius + (circleRadius-fanletRadius)*gAnimateScaler;
		curMinTheta = fanletMinTheta + (circleMinTheta - fanletMinTheta)*gAnimateScaler;
		curMaxTheta = fanletMaxTheta + (circleMaxTheta - fanletMaxTheta)*gAnimateScaler;
		drawFan(renderFlag, 0,0,0, curRadius,  curMinTheta ,curMaxTheta , 1.0 , T->doAttenuateBoundingShape,dark_green);
#endif
		}
	else
		{
		drawFan(renderFlag, 0,0,0, fanletRadius,  fanletMinTheta, fanletMaxTheta, 1.0 , T->doAttenuateBoundingShape, dark_green);
		if (!isTip(T)) // if there is yet another fanlet beneath this, just draw a fanlet-ita to act as a telltale
			{
			drawFan(renderFlag, fanletRadius,0,0, fanletRadius/2.,  fanletMinTheta, fanletMaxTheta, 1.0 , T->doAttenuateBoundingShape, dark_green);
			}
		}

	}
	
}

static void drawArcPetiole(DrawEnum renderFlag, OglTree t)
	{
	extern GLfloat yellow[];
	area A =(area)t->parentNode->data;
	float angle = A->theta_center;

	if (angle <= 0)
		{
		glRotatef(-90,0,0,1);
		glTranslatef(-t->anc->outerRadius, 0,0);
		drawArc(renderFlag,0,0,0,t->anc->outerRadius,0,-angle,FAN_OPACITY,t->doAttenuateBoundingShape,yellow);
		glRotatef(-toDegs(angle),0,0,1);
		glTranslatef(+t->anc->outerRadius, 0,0);
		}
	else
		{
		glRotatef(-90,0,0,1);
		glTranslatef(+t->anc->outerRadius, 0,0);
		drawArc(renderFlag,0,0,0,t->anc->outerRadius,PI,PI-angle,FAN_OPACITY,t->doAttenuateBoundingShape,yellow);
		glRotatef(-toDegs(angle),0,0,1);
		glTranslatef(-t->anc->outerRadius, 0,0);
		}
	
	}

static void drawArc(DrawEnum renderFlag, float x0, float y0, float z0, float r,  float theta1, float theta2, float opacity, short doAttenuate, GLfloat color[] )


// draws a filled arc in the z=z0 plane, with center at x0,y0.
// theta1,2 should be in radians!

// Need to work on the munging of opacity and darkness here within the attenuate code

	{
#define MAXARCSEG 100
#define MAXV 3*100
	int numCyl,numPts,ix=0,i; // determine adaptively!
	double v[MAXV],dth,theta,z,x,y;
	float x1,y1,z1,x2,y2,z2;
	float darkness;
	numCyl = fabs(100*(theta2-theta1)/TWO_PI); // poor adaptive calc for number of segs
	if (numCyl==0) numCyl=1;
	numPts=numCyl+1;
	dth = (theta2-theta1)/numCyl;	

//opacity=1;
	if (renderFlag==RENDERING)
		{
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		color[3]=opacity;
		glColor4fv(color);
		if (doAttenuate)
			{
			x1 = r * cos (theta1);
			y1 = r * sin (theta1);
			x2 = r * cos (theta2);
			y2 = r * sin (theta2); // I'm going to use as a surrogate for the fan, the line segment spanning the endpoints of the arc (doesn't handle panning offscreen vertically very well however! Need a box attenuator!f
			darkness = setColorWithAttenuationLineSegment(x1,y1,0,x2,y2,0, color, 15.0);
			}

		glDepthMask(GL_FALSE); // have to do this to prevent hidden line removal of objects behind the translucent floor of the box
		}
		
	glBegin(GL_LINE_STRIP);
	for (i=0;i<=numPts-1;i++)
		{
		theta = theta1 + dth*i;
		x = x0 + r * cos (theta);
		y = y0 + r * sin (theta);
		glVertex3d(x,y,z0);
		}
	glEnd();
	if (renderFlag==RENDERING)
		{
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		}
//printf("LAST POINT:%f %f %f %f %f %i\n",theta,r,z,x,y,numPts);




	}


// ***********************************
void drawFan(DrawEnum renderFlag, float x0, float y0, float z0, float r,  float theta1, float theta2, float opacity, short doAttenuate, GLfloat fanColor[] )


// draws a filled arc in the z=z0 plane, with center at x0,y0.
// theta1,2 should be in radians!

// Need to work on the munging of opacity and darkness here within the attenuate code

{
#define MAXARCSEG 100
#define MAXV 3*100
	int numCyl,numPts,ix=0,i; // determine adaptively!
	double v[MAXV],dth,theta,z,x,y;
	float x1,y1,z1,x2,y2,z2;
	float darkness;
	numCyl = fabs(100*(theta2-theta1)/TWO_PI); // poor adaptive calc for number of segs
	if (numCyl==0) numCyl=1;
	numPts=numCyl+1;
	dth = (theta2-theta1)/numCyl;	
    
    //opacity=1;
	if (renderFlag==RENDERING)
		{
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
		fanColor[3]=opacity;
		glColor4fv(fanColor);
		//		glColor4f(0,.15,0,opacity);
		
		//		GLfloat color[4]={0,0.15,0};
		if (doAttenuate)
		{
			x1 = r * cos (theta1);
			y1 = r * sin (theta1);
			x2 = r * cos (theta2);
			y2 = r * sin (theta2); // I'm going to use as a surrogate for the fan, the line segment spanning the endpoints of the arc (doesn't handle panning offscreen vertically very well however! Need a box attenuator!f
			darkness = setColorWithAttenuationLineSegment(x1,y1,z0,x2,y2,z0, fanColor, 15.0);
			}
			
		glDepthMask(GL_FALSE); // have to do this to prevent hidden line removal of objects behind the translucent floor of the box
		}
    
	glBegin(GL_TRIANGLE_FAN);
	glVertex3d(x0,y0,z0);
	for (i=0;i<=numPts-1;i++)
		{
			theta = theta1 + dth*i;
			x = x0 + r * cos (theta);
			y = y0 + r * sin (theta);
			
			glVertex3d(x,y,z0);
		}
	glEnd();
	if (renderFlag==RENDERING)
		{
			glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
		}
    //printf("LAST POINT:%f %f %f %f %f %i\n",theta,r,z,x,y,numPts);
    
    
    
    
}
//



// ***********************************
static void drawCactusPad(DrawEnum renderFlag, float a, float b, float opacity, short doAttenuate, GLfloat fanColor[] )

	{
	int numCyl,numPts,ix=0,i; // determine adaptively!
	double dth,theta,z,x,y;
	float x1,y1,z1,x2,y2,z2,theta1,theta2, r, e, a0;
	float darkness;
	numPts=101;
        theta1=0.0;
        theta2=TWO_PI;
    dth = (theta2-theta1)/(numPts-1);	

//opacity=1;
	if (renderFlag==RENDERING)
		{
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

fanColor[3]=opacity;
glColor4fv(fanColor);
#if 1
		if (doAttenuate) // REVISIT THIS
			{
			x1 = -a; // rMin is bogus; just temp
			y1 = 0;
			x2 = +a;
			y2 = 0; // I'm going to use as a surrogate for the fan, the line segment spanning the endpoints of the arc (doesn't handle panning offscreen vertically very well however! Need a box attenuator!f
			darkness = setColorWithAttenuationLineSegment(x1,y1,0,x2,y2,0, fanColor, 15.0);
			}
#endif
		glDepthMask(GL_FALSE); // have to do this to prevent hidden line removal of objects behind the translucent floor of the box
		}
		
	glBegin(GL_TRIANGLE_FAN);
	glVertex3d(0,0,0);
	for (i=0;i<=numPts-1;i++)
		{
		theta = theta1 + dth*i;
        //if (theta < PI/2 || theta > 3*PI/2) a0=b; else a0=a;
        a0=a;
        x =  a0 * cos (theta);
        y =  b * sin (theta);
		glVertex3d(x,y,0.0);
    // NB! THETA IS A PARAMETERIZATION FOR THE ELLIPSE. IT IS NOT THE ANGLE WITH THE X-AXIS
//    printf ("...%f %f %f %f\n",theta, a,b,ellipseRadius(theta,a,b));
        
		}
//printf("\n");

	glEnd();
	if (renderFlag==RENDERING)
		{
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		}
	}

static float nodeToEllipseDistance(OglTree T, node n, float a, float b)
{
    float x,y,r;
    area A;
    A=(area)n->data;
    r = ellipseRadius(A->theta_center, a, b) - T->radius;

    return r;
    
}

//
// ***********************************
static void drawCurvyCorner(DrawEnum renderFlag, float r,float opacity, short doAttenuate, GLfloat fanColor[] )



// Need to work on the munging of opacity and darkness here within the attenuate code

	{
#define MAXARCSEG 100
#define MAXV 3*100
	int numCyl,numPts,ix=0,i; // determine adaptively!
	double v[MAXV],dth,theta,z,x,y;
	float x1,y1,z1,x2,y2,z2;
	float darkness;
	float theta1 = 0.0, theta2 = PI/2;
	numCyl = fabs(100*(theta2-theta1)/TWO_PI); // poor adaptive calc for number of segs
	if (numCyl==0) numCyl=1;
	numPts=numCyl+1;
	dth = (theta2-theta1)/numCyl;	

//opacity=1;
	if (renderFlag==RENDERING)
		{
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


fanColor[3]=opacity;
glColor4fv(fanColor);
		if (doAttenuate) // UPDATE THIS LATER
			{
			x1 = r * cos (theta1);
			y1 = r * sin (theta1);
			x2 = r * cos (theta2);
			y2 = r * sin (theta2); // I'm going to use as a surrogate for the fan, the line segment spanning the endpoints of the arc (doesn't handle panning offscreen vertically very well however! Need a box attenuator!f
			darkness = setColorWithAttenuationLineSegment(x1,y1,0,x2,y2,0, fanColor, 15.0);
			}

		glDepthMask(GL_FALSE); // have to do this to prevent hidden line removal of objects behind the translucent floor of the box
		}
	glBegin(GL_TRIANGLE_FAN);
	glVertex3d(r,0,0); // have to start here...
	for (i=0;i<=numPts-1;i++)
		{
		theta = theta1 + dth*i;
//		x = r * cos (theta);
//		y = r * sin (theta);
		x = r - r * (1 - cos (theta)); // makes convex arc concave
		y = r * (1 - sin (theta));
		glVertex3d(x,y,0);
		}
	glEnd();
	if (renderFlag==RENDERING)
		{
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		}
//printf("LAST POINT:%f %f %f %f %f %i\n",theta,r,z,x,y,numPts);

	}

void drawCylinder(double x1, double y1, double z1, double x2, double y2, double z2, float radius1,float radius2)
// Draw a cylinder between arbitrary two points in xyz space (have to do our own trig here)

	{
	double dx,dy,dz,zr,xcross,ycross,theta;
	dx=x2-x1;
	dy=y2-y1;
	dz=z2-z1;
	zr = sqrt(dx*dx+dy*dy+dz*dz);
	xcross = -dy;
	ycross = +dx;
	theta = toDegs( acos (dz/zr)) ; 
	glPushMatrix();
	glTranslatef(x1,y1,z1);
	glRotatef(theta,xcross,ycross,0.0);
	gluCylinder(qobj,radius1,radius2,zr,10,1);
	glPopMatrix();
	}

/****************************************************************/

static void reshape (int w, int h) // keeps the tree with one aspect ratio regardless of resizing 
{
   float aspectRatio;
//int screenWidth,screenHeight;
//screenWidth = glutGet(GLUT_SCREEN_WIDTH);
//screenHeight=glutGet(GLUT_SCREEN_HEIGHT);
//printf("Screen H/W:%i %i\n",screenHeight,screenWidth);
   windowWidth=w;windowHeight=h; // store these in globals for use elsewhere
   aspectRatio=(float)w/h;
   glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity();
#define PERSPECTIVE 1.0
#if 1
   float persp = PERSPECTIVE;
   if (w <= h)
      glFrustum (-persp, persp, -persp/aspectRatio, persp/aspectRatio, FRONTCLIP,BACKCLIP);
   else
      glFrustum (-persp*aspectRatio, persp*aspectRatio, -persp, persp, FRONTCLIP,BACKCLIP);
#else
	float zoom = 4.0;
   float persp = PERSPECTIVE/zoom;
   if (w <= h)
      glFrustum (-persp/zoom, persp/zoom, -persp/aspectRatio/zoom, persp/aspectRatio/zoom, FRONTCLIP/zoom,BACKCLIP/zoom);
   else
      glFrustum (-persp*aspectRatio/zoom, persp*aspectRatio/zoom, -persp/zoom, persp/zoom, FRONTCLIP/zoom,BACKCLIP/zoom);
#endif


   glMatrixMode(GL_MODELVIEW);
if (fullScreen)
	glutFullScreen();
   return;
}


/****************************************************************/


static void printSceneTree(OglTree T)

	{
	char * rootstr="The root",*label;
	OglTree child,sib;
	if (T->anc) // can't retrieve parent for the root tree
		{
		if (isRoot(T->anc)) label = rootstr; else label=T->anc->root->label;
		printf("Traversing scene tree: tree %s leaves = %li parent tree label = %s\n",T->root->label,T->root->numdesc, label);
		
		}
	child = T->firstdesc;
//	SIBLOOP(child)
	if (child)
		printSceneTree(child);
	sib=T->sib;
	if (sib)
		printSceneTree(sib);
	}

float clamp0_360(float angle)
	{
	if (angle > 360) angle -= 360;
	if (angle < 0 ) angle += 360;
	return angle;
	}
float clamp0_TwoPI(float angle) // don't rename this TWO_PI! defined ...
	{
	if (angle > TWO_PI) angle -= TWO_PI;
	if (angle < 0 ) angle += TWO_PI;
	return angle;
	}
short inCircularInterval(float minX, float maxX, float x)
// Consider e.g. a circle in which angle goes from 0-360, and a pie where angle starts at minX and goes to maxX. Is x within pie?
// Have to consider weird case of e.g., pie from 350-10, which is obviously different from 10-350!
	{
	if (minX < maxX)
		if (minX <= x && maxX >= x) return 1; else return 0;
	if (minX >= maxX)
		if (minX <= x || maxX >= x) return 1; else return 0;
	}
	
	
OglTree findRootCircle(OglTree T) // moving rootward in scene tree, find the oldest circle, or return NULL if there are none
	{
	OglTree t=T;
	while (t->anc)
		{
		if (t->layout==circle && t->anc->layout !=circle) return t;
		t=t->anc;
		}
	return NULL;
	}

	
