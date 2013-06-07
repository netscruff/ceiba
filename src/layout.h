#pragma once
#include "mySmallTreeLib.h"
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
#include "label.h"
#include "branch.h"

// Really basic decisions about layout
#define CIRCLE_FRACTION 0.50
#define CACTUS 0
#define ECC_FACTOR 1.0 // when >1 this makes a tall narrow cladode


#define BW 0 // occasional black and white layouts

#define SQR(x) ((x)*(x))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define PI 3.141592
#define TWO_PI 6.2831853 
#define isCircle(a) ((a)->theta1==0.0 && (a)->theta2==TWO_PI && (a)->r1==0.0)

enum FlagTypes {DEFAULT_HIERARCHICAL_LAYOUT,ROTATE_TO_NODE, ROTATE_TO_TREE};
enum LayoutChoice {sphere, hemisphere,spiral,circle,cone,goblet,solid_cone, fanlet};
enum TerminalsChoice {terminals,terminals_internals,internals};
enum ImageType {PPM, STL, STLt, XYZ};
enum FaceStyle {TREE_FACES_VIEWER, TREE_FREE};
enum IntNameAlgo {USER_ONLY,TREE_ONLY, BOTH};

typedef struct oglTreeStruct * OglTree;
struct oglTreeStruct {
	OglTree		anc;
	OglTree		firstdesc;
	OglTree		sib;
	char		*label;
	long		numdesc;
	long		numdescEffective;
	long		numChildren;
	
    node		root;	// root of the phylogenetic tree, as opposed to the scene tree
	GLfloat		mv_matrix[16];
	GLfloat		mv_matrix_cumul[16];
	float		circleFraction;
	long		numSubclades;
	long		nLeaves;
    float		radius; // from root to leaf node
	float		outerRadius; // from root to leaf plus label
    float		height;

	enum LayoutChoice coordLayout;		// layout for which coordinates have been calculated
	enum LayoutChoice layout;			// currently displayed layout
	enum LayoutChoice defaultLayout;	// base display layout; always return to this
	enum FaceStyle faceStyle;
	float		treeXRotate;
	char*		maxLabel;		// pointer to longest label of this subtree's leaves
	float		maxLabelLength; // modeling unit length of longest terminal label in terminal label font
	LabelAttributeObj terminalLabelAttributes; // contains lots of info about labels
	LabelAttributeObj internalLabelAttributes; // contains lots of info about labels
	BranchAttributeObj branchAttributes;
	OglTree		parentTree; // parent tree
	node		parentNode; // node on the parent tree that is the root of this tree
	short		displayTerminalLabels;
	short		displayInternalLabels;
	short		doAttenuateBoundingShape; // for example, the fan of a circle layout
	short		animating;				// set to 1 if in the middle of animating this tree
	float		upDirection;		// the angle IN RADIANS on this circle which corresponds to the position pointing to the top of screen (well, after the 90 rotation)
	float		petioleLength;		// when a fanlet is expanded, this is the petiole length from rim of ancestor circle
	float		leafBaseLength;		// length along petiole axis of the leaf base if present
};
void oglTreeInit(OglTree t, node root, node parentNode, enum LayoutChoice layout, float radius, float height, float circleFraction, BranchAttributeObj branchAttributes, LabelAttributeObj terminalLabelAttributes,LabelAttributeObj intLabelAttributes, enum FaceStyle faceStyle, float treeXRotate, short displayTerminalLabels,short displayInternalLabels, short doAttenuateBoundingShape);
OglTree oglTreeAlloc(void);

typedef struct areaStruct * area;
struct areaStruct
	{
	double z;
	double r1;
	double r2;
	double theta1;
	double theta2;
	double r_center;
	double theta_center; // NB! this is in radians!
	double x_center; // the coordinates of the node
	double y_center;
	double x_anc; // these used in rendering code only
	double y_anc;
	double z_anc;
	double cross_prodx;
	double cross_prody;
//	double cross_prodz; always = zero for cylinders
	double zr;
	double theta; // NB! this is in degrees
	double rho;
	float x_disp;	// for force directed layout code
	float y_disp;
#if 1
	float z_disp;
#endif
	};
typedef struct areaPairStruct *areaPair;
struct areaPairStruct
	{
	area first;
	area second; // keep track of which area is smaller/larger
	};

float ellipseRadius(float theta, float a, float b);
void setRadius(node root, node a, float rootRadius, float leafRadius);
float maxLabelLengthGlut(node n, char **maxLabel, void * font); 
void setHierarchicalLayoutScheme(OglTree T, enum LayoutChoice layout[], enum FaceStyle face[], int array_size, int level);
void subtreeColor(node n, GLfloat *color);
void labelColorClade(node n, GLfloat *color);
void treeColorClade(node n, GLfloat *color);
OglTree setupTheTree(node a);
void descSplit(node a);
area areaNew(double r1, double r2, double theta1, double theta2);
void areaPrint(area A);
areaPair areaSplit(area A, unsigned long n1, unsigned long n2);
void treeAreaPrint(node a);
void treeSplit(node a);
void assignR(node a, double rHigh);
void assignZ(node a, double zLow, double zHigh);
unsigned long makeVertexArray(node a);
