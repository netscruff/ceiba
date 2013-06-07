#define VERSION 0.9 


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "my_structures.h"
#include "my_hash.h"
#include "mySmallTreeLib.h"
#include "tree_openGL.h"
//#include "layout.h"
#include <math.h>
#include <string.h>
#include "fdp.h"
#include "fonts.h"

// **************************

//enum LayoutChoice	layoutArray1[3] = {solid_cone,circle,fanlet};
//enum FaceStyle		faceArray1[3] = {TREE_FREE, TREE_FACES_VIEWER, TREE_FACES_VIEWER};

//enum LayoutChoice	layoutArray1[MAX_SCENE_TREE_DEPTH] = {solid_cone,circle,fanlet,fanlet,fanlet};
//enum FaceStyle		faceArray1[MAX_SCENE_TREE_DEPTH] = {TREE_FREE, TREE_FACES_VIEWER, TREE_FACES_VIEWER, TREE_FACES_VIEWER, TREE_FACES_VIEWER};
enum LayoutChoice      layoutArray1[MAX_SCENE_TREE_DEPTH];
enum FaceStyle         faceArray1[MAX_SCENE_TREE_DEPTH];

#define COMPACT_NUM_ITERATIONS 20 // used in setupCompactNodes

#define LABEL_COLOR white
#define TREE_COLOR my_brown
#define SUBTREE_COLOR yellow
#define SUBTREE_LABEL_COLOR white
#define TREE_STYLE 1		// 0 = lines ; 1 = cylinders
#define SUBTREE_STYLE 0

#if 1
#define MIN_CLADESIZE 10
#define MAX_CLADESIZE 100
#else
#define MIN_CLADESIZE 25
#define MAX_CLADESIZE 500

#endif


/*
The following control the minimum and maximum size of visible subclades, all relative to the size of the original bounding
box. Note that together with the minimum size of displayed text (in calcLabelScaleCircle), these sizes will control the look
and feel of labels within the subclades--such as how big and how much space between labels.
*/
#define MIN_SUBTREE_FRACTION 0.01
#define MAX_SUBTREE_FRACTION 0.10  // fraction of the parent trees radius that we allow for the subtree radius, modified by ntaxa in subtree



#define REPLACE_UNDERSCORES 0 // set if we want to display underscores as blanks -- careful, it mucks up reading file names correctly (remove and deal with in the drawLabel function
// **************************

// DIMENSIONS OF TREE LAYOUTS

#define PHYLOGRAM 1		// set to 1 for a phylogram-ish layout
#define CIRCLE_RADIUS 100	// initial size of all trees; will be scaled later as appropriate
#define HEMISPHERE_RADIUS 200
#define CONE_RADIUS 100

// **************************
#define SQUEEZE 1 // when 1 this distributes labels uniformly across sphere; if less then 1, squeezes diverse clades together more
#define ZCONST 0.5 // affects the relative height of nodes along the z- axis.
#define ST 0.04	// size of squares representing terminals 
#define HEMI_SPH 10  // set to 2 for sphere, 4 for hemisphere, when using that layout

GLfloat my_brown[]={0.25,.125,0.0, 1.0};
GLfloat grey[]={0.4,0.4,0.4, 1.0};

GLfloat brown[]={0.10,0.10,0.10,1.0};
GLfloat yellow[]={1.0,1.0,0.0,1.0};
GLfloat lt_yellow[]={0.8,0.8,0.0,1.0};
GLfloat red[]={1.0,0.0,0.0,1.0};
GLfloat lt_red[]={0.3,0.0,0.0,1.0};
GLfloat lt_green[]={0.0,0.2,0.0,1.0};
GLfloat green[]={0.0,1.0,0.0,1.0};
GLfloat white[]={1.0,1.0,1.0,1.0};
GLfloat black[]={0.0,0.0,0.0,1.0};
GLfloat blue[]={0.0,0.0,1.0,1.0};
GLfloat lt_blue[]={0.0,0.0,0.3,1.0};
GLfloat earth_blue[]={0.0,0.0,0.8,1.0};

char gFontLocation[256];
OglTree gRootOglTree;
char *rootStr = "Base tree";

Hash mrcaHash=NULL;     
Hash subclades=NULL;

float gAttractFactor = 5000; // good value for fdp() for ~25 leaves or clades visible..
#if 1
float gLeafRadius= 0.50;
float gRootRadius= 5.0;
#else
float gLeafRadius= 0.050;
float gRootRadius= .050;
#endif
int gHierarchical=1;
int gCompressImages=0;
int gMaxImagesPerNode=1;
enum ImageType gImageType;
int gIterFDP=25; // reasonable default
int gImages=0;
int gSqueezeLayout=1;
int gPhylo=0;
float gCircleFraction=1.0;
int gMode=0;
int genusMode=0;
enum LayoutChoice gLayoutChoice;
enum TerminalsChoice gTerminalsChoice;
double gZRange,gRadiusOfOriginalCircle,gViewerZ;
node gTreeRoot,gBaseTreeRoot;
float *gVertexArray;
float *gVertexArrayTips;

static void setHierarchicalCoordinateLayoutScheme(OglTree T) ;
static void initLayoutArray(int levels);
static void adjustLeafRForEllipse(node a, float ecc);
static long setupSubcladesSubordinate(OglTree theOglTree, node a, BranchAttributeObj branchAttributes, LabelAttributeObj terminalLabelAttributesTemplate, LabelAttributeObj internalLabelAttributes);
static long setupSubcladesFromCompactNodes(OglTree theOglTree, node a, BranchAttributeObj branchAttributes, LabelAttributeObj terminalLabelAttributesTemplate, LabelAttributeObj internalLabelAttributes);
static void setupCompactNodes(node root, int minsize, int maxsize, int maxlevels, enum IntNameAlgo);
static void setupCompactNodesHelper(node a, int minsize, int maxsize, enum IntNameAlgo);

static void internalNameCreator2(node a) ;
static void addChild(OglTree parent, OglTree theChild);
static void oglTreeLayout(OglTree t);
static float calcLabelScaleCircle(long n, float radius, float circleFraction);

static int initMarkedAnc(node n, node marked_node);
static float setSubtreeRadius(long nTree, long nSubtree, float rTree, float minRFraction, float maxRFraction, int nMin, int nMax);
static int subcladeMarkInit(node n);
static void internalNameCreator(node a);
static void treeStyleInit(node n, GLfloat *treeColor, int branchStyle, int replaceUnderscores);
//static long setupSubcladesFromTreeOnly_2(OglTree theOglTree, node a, BranchAttributeObj branchAttributes, LabelAttributeObj terminalLabelAttributes, LabelAttributeObj internalLabelAttributes, int minsize, int maxsize);
static void taxaPrint(node a);
unsigned long gVASize,gVASizeTips;
void printUsage(void);
static void assignZ_phylo(node a, double zLow, double factor); 
static void assignZ_unscaled2(node a, float zMin, float zMax);
static double treeLengthFlag(node n,int flag);
static double treeLengthFlagHelper(node n,int flag);
static void setupArcZ(node a);
static void internalNamesFromMRCACommand(node root);
static void setupArcCenters(node a, double *x);
static void setupCenters(node a, double *x, double *y);
static void scaleR(node a, float factor);
static void assignR_clado(node a, double r);
static void scaleZ(node a, float factor);
static float assignZ_unscaled(node a, long rootDist);
char * parseNodeName(char *label, char *separ, int jth);
char*  genusSetup(node a, int j);
static void xyz2polar(node a);


/**************************************************/
static void initLayoutArray(int levels)
       {
       int i;
       if (MAX_SCENE_TREE_DEPTH < 3)
               {
               printf("Max Scene Tree Depth was less than 3 -- WARNING\n");
               }

       for (i=0;i<MAX_SCENE_TREE_DEPTH;i++)
               {
			   
//^^^^^^^			   
                      layoutArray1[i]=solid_cone;
                       faceArray1[i]=TREE_FREE;
			   continue;
			   
			   
			   
			   
               if (i==0) 
                       {
                       layoutArray1[i]=solid_cone;
                       faceArray1[i]=TREE_FREE;
                       }
               if (i==1) 
                       {
                       layoutArray1[i]=circle;
                       faceArray1[i]=TREE_FACES_VIEWER;
                       }
               if (i>1) 
                       {
                       layoutArray1[i]=fanlet;
                       faceArray1[i]=TREE_FACES_VIEWER;
                       }



               }
       }


static void setHierarchicalDisplayLayoutScheme(OglTree T, enum LayoutChoice layout[], enum FaceStyle face[], int array_size, int level)

// Setups the visual hierarchy of subtrees. Note that this may not penetrate to lowest part of the tree if we only
// display the top few levels. For this reason, we don't use this routine to setup the coordinate layouts.


	{
	int index;
	if (level < 0 )  {printf ("Invalid level in setDisplayHierarchicalLayoutScheme\n"); exit(1); }; 
	if (level >= array_size)
		index = array_size -1 ;
	else
		index = level;
	// make sure to clamp the array index to [0, array_size-1]
	
	T->layout = layout[index];
	T->defaultLayout = layout[index];
	T->faceStyle = face[index];

	// Following code is inserted here for lack of a better place at the moment. Initializes current up direction on circles
	if (T->anc) 
		if (T->anc->layout == circle || T->anc->layout == fanlet)
		{
		area A;
		A = (area)(T->parentNode->data);
		//float parentNodeDirection = toDegs(A->theta_center);
		//T->upDirection = clamp0_360(T->anc->upDirection - parentNodeDirection);
		float parentNodeDirection = A->theta_center;
		T->upDirection = clamp0_TwoPI(T->anc->upDirection - parentNodeDirection);


//if (strcmp("Cynanchum, etc.", T->parentNode->label)==0)
//	printf ("...level %i: %f %f %f\n",level, T->upDirection ,T->anc->upDirection,parentNodeDirection);

		}


	if (T->firstdesc)
		setHierarchicalDisplayLayoutScheme(T->firstdesc,layout,face,array_size,level+1);
	if (T->sib)
		setHierarchicalDisplayLayoutScheme(T->sib,layout,face,array_size,level); // a'la Angel:433 depth first traversal
	

	}




static void setHierarchicalCoordinateLayoutScheme(OglTree T) 
	// 
	{
	OglTree child;
	if (T->anc == NULL)
		T->coordLayout = solid_cone;
	else
#if 0
		T->coordLayout = circle;
#else
		T->coordLayout = solid_cone;
#endif
	oglTreeLayout(T);
	child = T->firstdesc;
	SIBLOOP(child)
		setHierarchicalCoordinateLayoutScheme(child);

	}

	



static void addChild(OglTree parent, OglTree theChild)
        {
        OglTree aChild;
        if (parent->firstdesc)
            {
            aChild=parent->firstdesc;
            if (aChild)
                    {
                    while(aChild->sib)
                            aChild=aChild->sib;
                    aChild->sib=theChild;
                    }
            }
        else
            parent->firstdesc=theChild;
        theChild->anc=parent;
        theChild->sib=NULL;
		++parent->numChildren;
        return;
        }


char * parseNodeName(char *label, char *separ, int jth)

// Examines a label. Breaks the label into tokens based on the 'separ' character.
// If the label is empty (NULL ptr)  returns NULL. If the jth token exists, return a copy of it. If not return NULL. (1-offset thinking here)

	{
	char *word;
	if (label==NULL) return NULL;
	while (1)
		{
		word = strtok(DupStr(label),separ); // strtok modifies the string in place, so dup it
		if (--jth == 0) return word;
		while (word != NULL)
			{
			word = strtok(NULL,separ);
			if (--jth == 0) return word;
			}
		return NULL;
		}
	}

static void internalNameCreator(node a) 

/*
Makes up a name for a clade based on its leaf names.
Gets the first word in each leaf name, call this the "genus".
If all the leaves have the same genus, that's the clade name.
Otherwise, takes the first and last genus name in alphabetical order and puts them together name1...name2.

*/

	{
	char *s1, *s2, s[128], *genus1, *genus2;
#if REPLACE_UNDERSCORES
	char *delim=" ";
#else
	char *delim="_";
#endif
	nodeArray nodeArray;
	long numTaxa,i;
	if (isTip(a)) 
		return;
	numTaxa=a->numdesc;
	nodeArray=newTipNodeArray(a);

	qsort(nodeArray,numTaxa,sizeof(node),nodeLabelCompar);

	s1=(nodeArray[0])->label;
	genus1 = parseNodeName(s1,delim,1); // get the first token separated by underscore (maybe a genus name)

	s2=(nodeArray[numTaxa-1])->label;
	genus2 = parseNodeName(s2,delim,1); 
	

	if (strcmp(genus1,genus2)) // first and last "genera" are different
		{
		sprintf(s,"%s...%s",genus1, genus2);
		a->label = DupStr(s);
		}
	else // same genus names
		{
		sprintf(s,"%s",genus1);
		a->label = DupStr(s);
		}

	free(nodeArray);

	}


static void internalNameCreator2(node a) // what to do with non binomials...?
	{
	char *s, *genus, *s1, *s2, sb[128], *genus1, *genus2;
	Entry p,e;
	int i,*val;
	Hash genera = hashNew(25);

#if REPLACE_UNDERSCORES
	char *delim=" ";
#else
	char *delim="_";
#endif
	nodeArray nodeArray;
	long numTaxa;
	if (isTip(a)) 
		return;

	numTaxa=a->numdesc;
	nodeArray=newTipNodeArray(a);

	for (i=0;i<numTaxa;i++)
		{
		s=(nodeArray[i])->label;
		genus = parseNodeName(s,delim,1); // get the first token separated by underscore (maybe a genus name)
		if (e=hashKeyExists(genera,genus))
			{
			++*((int*)e->val);
			}
		else
			{
			val = (int *)malloc( sizeof (int) );
			*val=1;
			hashInsert(genera,genus,val,&e);
			}
		}

	//qsort(nodeArray,numTaxa,sizeof(node),nodeLabelCompar);

	

	free(nodeArray);




	int size,maxsize=0;
	char* savegenus;

	for (i=0;i<genera->numElements;i++)
			{
			p=hashGetKthEntry(genera,i);
			genus = p->key;
			size = *((int*)p->val);
			if (size>maxsize)
				{
				maxsize=size;
				savegenus=genus;
				}
			//printf("Processing genus name %s\n",genus);
			}
	//printf("Max genus = %s (%i)\n",savegenus,maxsize);
	if (genera->numElements == 1)
		{
		sprintf(sb,"%s",savegenus);
		a->label = DupStr(sb);
		}
	if (genera->numElements > 1)
		{
		sprintf(sb,"%s, etc. (%3li gen.)",savegenus,genera->numElements);
		sprintf(sb,"%s, etc.",savegenus);
		a->label = DupStr(sb);
		}
	}






char*  genusSetup(node a, int jth)

// sets up internal node sames by parsing terminal node names into, e.g., "genera" and whenever ALL descendents
// of a node have the same genus name, storing that name at the internal node for later display as a higher taxon.

	{
	char *genus0, *genus1;
	node child;
	int failed=0;
	if (isTip(a))
		{
		genus0 = parseNodeName(a->label,"_",jth); // the genus better be the jth token...
		return genus0;
		}
	child = a->firstdesc;
	genus0 = genusSetup(child,jth);
	if (genus0 == NULL) ++failed;
	child = child->sib;
	SIBLOOP(child)
		{
		genus1 = genusSetup(child,jth);
		if (genus0)
			{
			if (genus1 == NULL) 
				++failed;
			else
			    	if (strcmp(genus0,genus1) != 0)
					++failed;
			}
		}	
	if (failed > 0)
		a->label=NULL;
	else
		{
		a->label=genus0;
		child = a->firstdesc; // useful to reset its (nontip) children to NULL, so nested views with defined
					// higher clades can be combined with genus view
		SIBLOOP(child)
			if (!isTip(child))
				child->label=NULL;
		}
	return a->label;
	}

unsigned long makeVertexArray(node a) // returns the number of vertices stored in the array (each vertex having three coords)
	{
	static int startFlag=1;
	static unsigned long ix=0,nn;
	node child,anc;
	area A,ancA;
	float x1,x2,y1,y2,z1,z2;
	if (startFlag)
		{
		nn=numNodes(a)-1;  // there's no branch below the root, so we ignore the root node everywhere below
		gVertexArray=(float*)malloc(sizeof(float)*6*nn);
		startFlag=0;
		}
	if (!isRoot(a))
		{
		A=(area)(a->data);
		ancA=(area)(a->anc->data);
		gVertexArray[ix++]=A->x_center;
		gVertexArray[ix++]=A->y_center;
		gVertexArray[ix++]=A->z;
		gVertexArray[ix++]=ancA->x_center;
		gVertexArray[ix++]=ancA->y_center;
		gVertexArray[ix++]=ancA->z;
		}
	child=a->firstdesc;
	SIBLOOP(child)
		makeVertexArray(child);
	return nn*2;
	}
unsigned long makeVertexArrayTips(node a) // returns the number of vertices stored in the array (each vertex having three coords)
	{
	static int startFlag=1;
	static unsigned long ix=0,nn;
	node child;
	area A;
	float x,y,z;
	if (startFlag)
		{
		nn=numdesc(a);  // there's no branch below the root, so we ignore the root node everywhere below
		gVertexArrayTips=(float*)malloc(sizeof(float)*4*nn*3);
		startFlag=0;
		}
	if (isTip(a))
		{
		A=(area)(a->data);
		x=A->x_center;
		y=A->y_center;
		z=A->z;
		gVertexArrayTips[ix++]=x+ST;
		gVertexArrayTips[ix++]=y;
		gVertexArrayTips[ix++]=z;

		gVertexArrayTips[ix++]=x;
		gVertexArrayTips[ix++]=y+ST;
		gVertexArrayTips[ix++]=z;

		gVertexArrayTips[ix++]=x-ST;
		gVertexArrayTips[ix++]=y;
		gVertexArrayTips[ix++]=z;

		gVertexArrayTips[ix++]=x;
		gVertexArrayTips[ix++]=y-ST;
		gVertexArrayTips[ix++]=z;

		}
	child=a->firstdesc;
	SIBLOOP(child)
		makeVertexArrayTips(child);
	return nn*4;
	}
area areaNew(double r1, double r2, double theta1, double theta2)
	{
	area A;
	A = (area)malloc(sizeof(struct areaStruct));
	A->r1=r1;
	A->r2=r2;
	A->theta1=theta1;
	A->theta2=theta2;
	if (isCircle(A)) // note this is an important layout decision to always put center of circle at center
		A->r_center=0.0;
	else
		A->r_center = 0.5*(r1+r2);
	A->theta_center = 0.5*(theta1+theta2); // the default value, will stay true for terminals, not for other nodes
	A->x_center = A->r_center * cos(A->theta_center);
	A->y_center = A->r_center * sin(A->theta_center);
	A->x_disp=0;
	A->y_disp=0;
	return A;
	}
void areaPrint(area A)
	{
	printf("Printing area:\n");
	printf("\tr1             ...%f\n",A->r1);
	printf("\tr2             ...%f\n",A->r2);
	printf("\ttheta1         ...%f\n",A->theta1);
	printf("\ttheta2         ...%f\n",A->theta2);
	printf("\tr_center       ...%f\n",A->r_center);
	printf("\ttheta_center   ...%f\n",A->theta_center);
	printf("\tx_center       ...%f\n",A->x_center);
	printf("\ty_center       ...%f\n",A->y_center);
	printf("\tz              ...%f\n",A->z);
	}

areaPair areaSplit(area A, unsigned long n1, unsigned long n2)

	// split the area in two such that the smaller area has a fraction n_small/(n_small+n_large) of the total area
	// The ordering of smaller and larger sizes is important because I try to keep smaller areas toward the center
	// to prevent large arcs with narrow radial axes
	// Algorithm imitates "ladderize" option in 2D

	{
	long n_small, n_large;
	areaPair P;
	area A1,A2;
	double f,a,b,r1,r2,theta1,theta2,theta1prime,theta2prime,rp;
	double dr,dth,aspect,aspect1,aspect2,cutoffAspect=5.0,Delta,fract=0.5;
	P=(areaPair)malloc(sizeof(struct areaPairStruct));
	if (n1<n2)
		{n_small=n1;n_large=n2;}
	else
		{n_small=n2;n_large=n1;}
	f=(double)n_small/(n_small+n_large);
f=pow(f,SQUEEZE);
	r1=A->r1;
	r2=A->r2;
	theta1=A->theta1;
	theta2=A->theta2;
	dr = r2-r1; 
	dth=r2 *(A->theta2 - A->theta1);
	aspect = dr/dth; // aspect ratio > 1 is long radial arms; < 1 is long arcs 
	while (aspect > cutoffAspect) // keeps some aspect ratios of some areas in bounds
				{
				Delta = fract*(r2-r1)/2; 
				rp = r1 + (r2-r1)/2; // midpoint
				r1= rp-Delta;
				r2= rp+Delta;
				A->r1 = r1;
				A->r2 = r2;
				dr = A->r2 - A->r1;
				dth=A->r2 *(A->theta2 - A->theta1);
				aspect = dr/dth;
				}
#if 0   // the following case seems necessary to cover the other kind of extreme aspect ratio, but it generates
	// one-sided trees! work on it later
	while (aspect < 1/cutoffAspect)
				{
				Delta = fract*(theta2-theta1)/2; 
				rp = (theta2-theta1)/2;
				theta1= rp-Delta;
				theta2= rp+Delta;
				A->theta1 = theta1;
				A->theta2 = theta2;
				dr = A->r2 - A->r1;
				dth=A->r2 *(A->theta2 - A->theta1);
				aspect = dr/dth;
				}
#endif
	a= (A->r1)*(A->r1);
	b= (A->r2)*(A->r2);
	if (aspect > 1.0)
				{

				r1=A->r1;
				r2=sqrt(f*b+(1-f)*a);
				theta1=A->theta1;
				theta2=A->theta2;
				aspect1 = (r2-r1)/(r2*(theta2-theta1));
				A1=areaNew(r1,r2,theta1,theta2);		
				r1=r2;
				r2=A->r2;
				aspect2 = (r2-r1)/(r2*(theta2-theta1));
				A2=areaNew(r1,r2,theta1,theta2);		
				}
	else
				{					
				r1=A->r1;
				r2=A->r2;
				theta1=A->theta1;
				theta2=theta1 + f*(A->theta2-theta1);
				A1=areaNew(r1,r2,theta1,theta2);
				aspect1 = (r2-r1)/(r2*(theta2-theta1));
				theta1=theta2;
				theta2=A->theta2;
				A2=areaNew(r1,r2,theta1,theta2);
				aspect2 = (r2-r1)/(r2*(theta2-theta1));
				}
	if (n1<n2)
		{P->first=A1; P->second=A2;}
	else
		{P->first=A2; P->second=A1;}
	return P;
	}
static void taxaPrint(node a)
	{
	node child;
	if(a->label)
		{
		printf("%s\n",a->label);
		return;
		}
	child=a->firstdesc;
	SIBLOOP(child)
		taxaPrint(child);
	return;
	}
void treeAreaPrint(node a)
	{
	node child;
	if(isTip(a))
		printf("\nTip Area:%s\n",a->label);
	areaPrint(a->data);	
	child=a->firstdesc;
	SIBLOOP(child)
		treeAreaPrint(child);
	return;
	}
void treeSplit(node a)
	{
	node child;
	//if (isTip(a)  || (a->label && gTerminalsChoice == internals))
	if (isTip(a)  || a -> isCompactNode)
		return;
	descSplit(a);
	child=a->firstdesc;
	SIBLOOP(child)
		treeSplit(child);
	return;
	}

static void setupArcCenters(node a, double *theta_c)
	{
	extern double gZRange;
	node child;
	area A;
	double X=0.0,Y=0.0,xc,yc,tc,thetaCenter=0.0;
	unsigned long N=0;
	//if (isTip(a)    || (a->label && gTerminalsChoice == internals))
	if (isTip(a)  || a -> isCompactNode)
		{
		A=(area)(a->data);
		*theta_c = A->theta_center;
		}
	else
		{
		child=a->firstdesc;
		SIBLOOP(child)
			{
			setupArcCenters(child,&tc);
			thetaCenter+=tc;
			++N;
			}
		*theta_c=thetaCenter/N;
		A=(area)(a->data);
		A->theta_center=*theta_c;
		}
	return;
	}
static void setupArcZ(node a) // assigns the z values for spiral and circular layouts; must be called after setupArcCenters...
	{
	extern double gZRange;
	extern enum LayoutChoice gLayoutChoice;
	node child;
	area A;
	A=(area)(a->data);
	switch (gLayoutChoice)
		{
		case circle: A->z = 0; /*A->theta_center*gZRange;*/ break; 
		case spiral: A->z = A->theta_center*gZRange; break; 
		case cone: A->z = A->r1 ; break;
		case goblet: A->z = A->r1 ; 
		}
//if (a->label && gTerminalsChoice == internals) return;
if (isTip(a)  || a -> isCompactNode) return;
	child=a->firstdesc;
	SIBLOOP(child)
		setupArcZ(child);
	}
static void setupCenters(node a, double *x, double *y)
	{
	node child;
	area A;
	double X=0.0,Y=0.0,xc,yc;
	unsigned long N=0;
	//if (isTip(a)    || (a->label && gTerminalsChoice == internals))
	if (isTip(a)    || a ->isCompactNode)
		{
		A=(area)(a->data);
		*x = A->x_center;
		*y = A->y_center;
		}
	else
		{
		child=a->firstdesc;
		SIBLOOP(child)
			{
			setupCenters(child,&xc,&yc);
			X+=xc;
			Y+=yc;
			++N;
			}
		*x=X/N;
		*y=Y/N;
		A=(area)(a->data);
		A->x_center=*x;
		A->y_center=*y;
		}
	return;
	}

// Following scaling routines used in circular and spiral code

void assignR(node a, double rHigh)
// Assigns r values to the nodes scaled from 0.0 to rHigh.

	{
	float maxLength,factor;
	
	switch (gPhylo)
		{
		case 0: 
			assignR_clado(a,rHigh);
			break;
		case 1:
			maxLength = calcMaxToTip(a);  // the longest distance from root to tip
			factor=rHigh/maxLength;
			assignR_clado(a,factor);
			break;
		case 2:
			maxLength = maxorder(a);  // the longest distance from root to tip when lengths are 1 is just the maxorder number!
			factor=rHigh/maxLength;
			assignR_clado(a,factor);
			break;
		}
#if 0	
	if (gPhylo)
		{
		maxLength = calcMaxToTip(a);  // the longest distance from root to tip
		factor=rHigh/maxLength;
		assignR_clado(a,factor);
		}
	else
		{
		assignR_clado(a,rHigh);
		}
#endif
	}
static void scaleR(node a, float factor) // since r1=r2, we just use r1
	{
	node child;
	area A;
	A = (area)(a->data);
        A->r1 *= factor;
	A->x_center = A->r1 * cos(A->theta_center);
	A->y_center = A->r1 * sin(A->theta_center);
if (a->label && gTerminalsChoice == internals) return;
        child=a->firstdesc;
        SIBLOOP(child)
                scaleR(child,factor);
	}
static void adjustLeafRForEllipse(node a, float ecc) // since r1=r2, we just use r1
	{
	node child;
    float ellipseRad,newR;
	area A;
	A = (area)(a->data);
    if (isTip(a)  || a -> isCompactNode)
        {
        newR = ellipseRadius(A->theta_center, ecc*A->r_center, A->r_center);
//printf ("%s %f %f %f %f\n",a->label,A->theta_center, A->r_center, ecc*A->r_center,newR);
        A->x_center = newR * cos(A->theta_center);
        A->y_center = newR * sin(A->theta_center);
        return;
        }
    child=a->firstdesc;
    SIBLOOP(child)
            adjustLeafRForEllipse(child,ecc);
	}
float ellipseRadius(float theta, float a, float b) // semi-ellipse!
{
    float r;


//    if (clamp0_TwoPI(theta) < PI/2 || clamp0_TwoPI(theta) > 3*PI/2) // half circle
//        r=b;
 //   else
        r = a*b/sqrt(SQR(b*cos(theta))+SQR(a*sin(theta))); // half ellipse

    return r;
    
}

static void assignR_clado(node a, double maxR) // determines branch length layout, here "cladogram" scaled by maxR
	{
	static float maxr = 0.0,r1;
	static long treeOrder;
	static double segment;
	node child,anc;
	area A,Aanc;
	double r;
	if (isRoot(a))
		{
		treeOrder=pow(a->order,(double)gSqueezeLayout);
		//treeOrder=a->order;
		segment = maxR/treeOrder;
		r=0.0;
		}
	else
		{
		if (gPhylo)
			{
			anc = a->anc;
			Aanc=((area)(anc->data));
			if (gPhylo == 1)
				r = maxR*a->number + Aanc->r1;
			if (gPhylo == 2)
				r = maxR + Aanc->r1;	// just set length to 1 in this mode
			}
		else
			{
			//if (isTip(a) || (a->label && gTerminalsChoice == internals))
			if (isTip(a)  || a -> isCompactNode)
				r =maxR;
			else
				r=(treeOrder- pow(a->order,(double)gSqueezeLayout))*segment;
			//	r = (treeOrder - a->order)*segment;
			//	r = (treeOrder - a->order*a->order)*segment;
			}
		}
	A=((area)(a->data));
	A->r1=r;
	A->x_center = A->r1 * cos(A->theta_center);
	A->y_center = A->r1 * sin(A->theta_center);
//if (a->label && gTerminalsChoice == internals) return;
	if (isTip(a)  || a -> isCompactNode) return;
	child=a->firstdesc;
	SIBLOOP(child)
		assignR_clado(child,maxR);
	return ;
	}

// Following scaling routines used in hemisphere code




void assignZ(node root, double zLow, double zHigh)
	// Assigns z values to the nodes scaled from zLow to zHigh.

	// Notice that this assigns z value to node's area structure, which means those areas must be created first!
	{
			float branch_length;

			// This means all branches have the same length
			branch_length = (zHigh-zLow)/root->order;
			assignZ_phylo(root,zLow,branch_length);
			return;



#if PHYLOGRAM	// if 1, set for hemisphere labels do NOT appear on surface of globe, but instead more like phylogram...
	//maxz = assignZ_unscaled(a,0);
	//scaleZ(a,zHigh/maxz); // multiply all z-values by this fraction to scale them all to zHigh
#else  // DISABLED RIGHT NOW because PHYLOGRAM IS SET TO 1
	if (gPhylo)
		{
		maxz = calcMaxToTip(a);  // the longest distance from root to tip
		assignZ_phylo(a,zHigh/maxz);
		}
	else
		assignZ_unscaled2(a,0,zHigh);
#endif
	}


//
// Phylogram display for cone or hemi using branch lengths  PRESENTLY FORCING ALL BRANCH LENGTHS ARE EQUAL!
//
static void assignZ_phylo(node a, double zLow, double brlen) 
	{
	node child,anc;
	area A,Aanc;
	double z;
	A=((area)(a->data));
	if (isRoot(a))
		A->z=zLow;
	else
		{
		anc = a->anc;
		Aanc=((area)(anc->data));
//		A->z=brlen*a->number+Aanc->z;
		A->z=brlen          +Aanc->z;
		}
//printf("factor=%f branch length=%f z=%f\n",brlen,a->number,A->z);
//	if (a->label && gTerminalsChoice == internals) return;
	if (a->isCompactNode) return;
	child=a->firstdesc;
	SIBLOOP(child)
		assignZ_phylo(child,zLow,brlen);
	return ;
	}







static void scaleZ(node a, float factor)
	{
	node child;
        ((area)(a->data))->z *= factor;
        child=a->firstdesc;
        SIBLOOP(child)
                scaleZ(child,factor);
	}

static float assignZ_unscaled(node a, long rootDist)
	{
	static float maxz = 0.0,z;
	static long treeOrder;
	node child;
	if (isRoot(a))
		{
		treeOrder=a->order;
		z = 0.0;
		}
	else
	  	z = ((area)(a->anc->data))->z + (float)(treeOrder-rootDist)/treeOrder; // rootDist is the edges to the root
	((area)(a->data))->z=z;
	if (z>maxz) 
		maxz=z;
	child=a->firstdesc;
	SIBLOOP(child)
		maxz=assignZ_unscaled(child,rootDist+1);
	return maxz;
	}
static void assignZ_unscaled2(node a, float zMin, float zMax)
        {
        static float maxz = 0.0,z;
        static long treeOrder;
        node child;
        static double segment;
        if (isRoot(a))
		{
		z=zMin;
		treeOrder=pow(a->order,(double)gSqueezeLayout);
		segment = zMax/treeOrder;
		}
        else    
                {
                if (isTip(a) || (a->label && gTerminalsChoice == internals))
                        z =zMax;
                else
                        //z = zMin + (zMax - zMin)/(float)(a->order + 1);
			z=(treeOrder- pow(a->order,(double)gSqueezeLayout))*segment;
                }
        ((area)(a->data))->z=z;
        child=a->firstdesc;
        SIBLOOP(child)
                assignZ_unscaled2(child,zMin,zMax);
        return;
        }

static void xyz2polar(node a)
	{
	node child;
	double v, w, r, x, y, z;
	area A;
	A=(area)(a->data);
	x=A->x_center;
	y=A->y_center;
	z=A->z;
	A->r_center = sqrt (x*x+y*y);
	A->theta_center = atan2(y,x);

	child=a->firstdesc;
	SIBLOOP(child)
		xyz2polar(child);
	return;
	}

void coneToHemiSphere(node a, double radius) 
// Takes the x,y,z coordinates in a cone structure and expands them to a hemisphere (or sphere, etc).
// This makes the final radius of the hemisphere dependent on the z values therefore, which is why
// the hemisphere drawing code sets these z-values first in a prior call.
// radius should generally correspond to radius of the original untransformed circle area (e.g., 1.0)
	{
	node child;
	double v, w, r, x, y, z;
	area A;
	A=(area)(a->data);
	r = A->z;
	w = TWO_PI*A->r_center/(HEMI_SPH*radius); // HEMI_SPH=2 for sphere, 4 for hemisphere
	v = A->theta_center; 

	x= r*sin(w)*cos(v);
	y= r*sin(w)*sin(v);
	z= r*cos(w);

	A->x_center=x;
	A->y_center=y;
	A->z=z;

	child=a->firstdesc;
	SIBLOOP(child)
		coneToHemiSphere(child, radius);
	return;
	}


void descSplit(node a) // sets up the areas of the immediate descendants of node a
			// Note how this depends greatly on whether the node is treated as a clade (with its "effective" N)
			// or as just a node with proper descendants
	{
	node n;
	area A0;
	areaPair P;
	unsigned long n1,n2,n0;
	if (isTip(a))
		return;
	n=a->firstdesc;
	A0=a->data;
	n0=a->numdescEffective;
	while (n->sib)
		{
		if (n->isCompactNode || isTip(n))
			n1=1;
		else
			n1=n->numdescEffective;
		n2=n0-n1;
		P=areaSplit(A0,n1,n2);
		n->data=P->first;
		A0=P->second;
		n0=n2;
		n=n->sib;
		}
	n->data=A0;
	}

static double treeLengthFlag(node n, int countRoot)
/* Sums the branch lengths over tree assuming they are stored in the 'number' field, only for branches of subtree indicated
   by nodeFlag=1. A previous call to subtreeLight will set this flag for a subtree of the tree, and so we can expect that
   that subtree will have an mrca node that we will encounter 'first' in the preorder traversal. There is where we can
   choose whether to count its subtending branch or not, and there is a little logical complexity in how this is done...
   */
{
if (countRoot==1)
	return treeLengthFlagHelper(n,1); 
else
	return treeLengthFlagHelper(n,0);
	// setting to 1 will fool the helper into always counting the length for every node, including the root node; setting to 0 will force it to ignore the first encounter with the mrca of the subtree 
}
static double treeLengthFlagHelper(node n, int foundFirstYet)

{
	node child;
	double dur;
	if (n->nodeFlagSave)
		{
		if (foundFirstYet==1)
			dur=n->number;
		else
			{
			dur=0.0;
			foundFirstYet=1;
			}
		}
	else
		dur=0.0;
	child=n->firstdesc;
	SIBLOOP(child)
		dur+=treeLengthFlagHelper(child,foundFirstYet);
	return dur;
}

void labelColorClade(node n, GLfloat *color)

// NB! The tree data structure just has float* arrays for color, but these routines want GLfloat* arrays...seems OK.

/* color is a 4-float array of RGB values that has been permanently allocated somewhere prior to calling;
   a pointer to this value will be copied to all nodes in the clade defined by node n */

{
	node child;
	n->labelColor=color;
	child=n->firstdesc;
	SIBLOOP(child) 
		labelColorClade(child,color); 
}

void subtreeColor(node n, GLfloat *color)
// if the node's nodeFlagSave is set, color it this way...
{
	node child;
//	if (n->nodeFlagSave)
		n->treeColor=color;
	child=n->firstdesc;
	SIBLOOP(child) 
		subtreeColor(child,color); 
}
void treeColorClade(node n, GLfloat *color)
// color the whole clade this way
{
	node child;
	n->treeColor=color;
	child=n->firstdesc;
	SIBLOOP(child) 
		treeColorClade(child,color); 
}

static void treeStyleInit(node n, GLfloat *treeColor, int branchStyle, int replaceUnderscores)
// init all nodes in the tree with these parms and maybe replace underscores
{
	node child;
	n->treeColor=treeColor;
	n->branchStyle=branchStyle;
	if (replaceUnderscores && n->label)
		replace_underscore(n->label);
	
	child=n->firstdesc;
	SIBLOOP(child) 
		treeStyleInit(child,treeColor,branchStyle,replaceUnderscores); 
}

static int subcladeMarkInit(node n)
// mark all nodes in the tree which have a descendant that is a compact node; assumes newnode inits marked=0
{
	node child;
	if (n->isCompactNode)
		{
		n->marked=1;
		return 1;
		}
	child=n->firstdesc;
	SIBLOOP(child) 
		if (subcladeMarkInit(child))
			{
			n->marked=1;
			}
	return n->marked;
}

// DONT NEED THIS ANY MORE
static int initMarkedAnc(node n, node marked_node)
// At every leaf node write the address of its closest marked ancestor (or leave null, if none). Call this with NULL as second arg.
// Note this does not yet get copied over to any subtrees that are defined later...
{
	node child;
	if (isTip(n))
		{
		n->markedAnc = marked_node;
		return;
		}
	else
		{
		if (n->isCompactNode)
			marked_node=n;
		child=n->firstdesc;
		SIBLOOP(child) 
			initMarkedAnc(child,marked_node);
		}
}

void setRadius(node root, node a, float rootRadius, float leafRadius)
// Setup radius field at each node. This will linearly interpolate between root and leaf radii

{
	node child;
	float delta, radius;
	delta = (rootRadius - leafRadius)/(root->order);
	a->radius = leafRadius + delta *(a->order);
	child=a->firstdesc;
	SIBLOOP(child) 
		setRadius(root,child,rootRadius,leafRadius); 
}


// ******************************************************************************************************************************
// ******************************************************************************************************************************

main(int argc,char * argv[])

{
	OglTree theOglTree;
	node root;
	enum IntNameAlgo intNameAlgorithm = TREE_ONLY;
	extern enum LayoutChoice gLayoutChoice;
	extern enum TerminalsChoice gTerminalsChoice;
	char *buffer,*p, buf1[512], buf2[512],*fnInput,*layoutStr,**sargv,*dummy;
	FILE *f;
	extern float gCircleFraction;
	int c, parseIndex=1,PD=0;
	gTerminalsChoice=terminals;
	//gTerminalsChoice=internals;
	sargv=argv;
	fnInput=layoutStr=NULL;


	char fontName[128]="Arial"; //default
	char fontLocation[]="/Library/Fonts/"; // fixed at the moment for mac!


	if (argc == 1)
		{
		printUsage();
		return;
		}
	else
		{
		c=0;
		while (++c<=argc-1)
			{
			p=argv[c];

			// options with arguments:
			// ..........................................................
			// ..........................................................
			
			if (strcmp(p,"-f")==0)
			
				{
				if (c+1 <= argc-1) // check there is at least another arg
					{
					++c;
					strcpy(buf1, argv[c]);   /* set file name */
					fnInput=buf1;
					if (  !(f=fopen(fnInput,"r")) )
						{
						printf("Error opening %s\n", fnInput);
						exit(1);
						}
					else
						fprintf(stderr, "[...reading file %s]\n", fnInput);
					}
					
				else
					fatal("Expecting another command line argument");
				continue;
				}
				
			// ..........................................................
				
			if (strcmp(p,"-t")==0)
			
				{
				if (c+1 <= argc-1)
					{
					++c;
					if (strcmp("ppm",argv[c]) == 0)
						gImageType = PPM;
						
					if (strcmp("stl",argv[c]) == 0)
						gImageType = STL;
					if (strcmp("stlt",argv[c]) == 0)
						gImageType = STLt;
					if (strcmp("xyz",argv[c]) == 0)
						gImageType = XYZ;
					gImages=1;
					}
					
				else
					fatal("Expecting another command line argument");
				continue;
				}
				
			// ..........................................................
				
			if (strcmp(p,"-I")==0)
			
				{
				if (c+1 <= argc-1)
					{
					++c;
					if (isdigit(*argv[c]))
						gIterFDP=strtod(argv[c],&dummy); // if a valid number convert it, else leave as default
					}
					
				else
					fatal("Expecting another command line argument");
				continue;
				}

			// ..........................................................
				
			if (strcmp(p,"-max_images")==0)
			
				{
				if (c+1 <= argc-1)
					{
					++c;
					if (isdigit(*argv[c]))
						gMaxImagesPerNode=strtod(argv[c],&dummy); // if a valid number convert it, else leave as default
					}
					
				else
					fatal("Expecting another command line argument");
				continue;
				}
			// ..........................................................

			if (strcmp(p,"-x")==0)
			
				{
				if (c+1 <= argc-1)
					{
					++c;
					if (strcmp("user_only",argv[c]) == 0)
						intNameAlgorithm = USER_ONLY;
					if (strcmp("tree_only",argv[c]) == 0)
						intNameAlgorithm = TREE_ONLY;
					if (strcmp("both",argv[c]) == 0)
						intNameAlgorithm = BOTH;
					}
					
				else
					fatal("Expecting another command line argument");
				continue;
				}

			if (strcmp(p,"-font")==0)
			
				{
				if (c+1 <= argc-1)
					{
					++c;
					strcpy(fontName, argv[c]);   /* set file name */
					}
					
				else
					fatal("Expecting another command line argument");
				continue;
				}

			// options without arguments:
			// ..........................................................

			if (strcmp(p,"-v")==0)
				{
				printf("ceiba version %4.2f (%s)\n",VERSION,__DATE__);
				exit(1);
				}

			if (strcmp(p,"-h")==0)
				{
				printf("ceiba version %4.2f (%s)\n",VERSION,__DATE__);
				printUsage();
				exit(1);
				}

			if (strcmp(p,"-d")==0)
				{
				gHierarchical = 0; // turn OFF hierarchical layout
				continue;
				}

			if (strcmp(p,"-compress_images")==0)
				{
				gCompressImages = 1; // use texture compression for images
				printf("Using texture compression...\n");
				continue;
				}


			// ..........................................................


			}
		}


		sprintf(gFontLocation,"%s%s.ttf",fontLocation,fontName);
		printf("Loading font %s\n",gFontLocation);


		
// ...should check to avoid displaying phylogram and internal nodes... 
	buffer = slurpNexus(f);
	root=nexus2rootNode(buffer); // Actually sets up a tree data structure from input file and calls to mySmallTreeLib 

// Legacy stuff
	if (layoutStr)
		{
		if (!strcmp(layoutStr,"hemisphere")) gLayoutChoice=hemisphere;
		if (!strcmp(layoutStr,"h")) gLayoutChoice=hemisphere;
		if (!strcmp(layoutStr,"solid_cone")) gLayoutChoice=solid_cone;
		if (!strcmp(layoutStr,"sc")) gLayoutChoice=solid_cone;
		if (!strcmp(layoutStr,"circle")) gLayoutChoice=circle;
		if (!strcmp(layoutStr,"cone")) gLayoutChoice=cone;
		if (!strcmp(layoutStr,"goblet")) gLayoutChoice=goblet;
		if (!strcmp(layoutStr,"spiral")) gLayoutChoice=spiral;
		}
	else
		gLayoutChoice = solid_cone; // current default


	font_init();
	
	initLayoutArray(MAX_SCENE_TREE_DEPTH);

	if (genusMode)
		{
		(void)genusSetup(root,parseIndex);
//		taxaPrint(root); enable this if you want to print out names of all recognized genera plus other terminals
		}


	LabelAttributeObj terminalLabelAttributes1;
	terminalLabelAttributes1 = labelAttributeObjAlloc(); //horizontal bitmap terminals
	labelAttributeObjInit(
							terminalLabelAttributes1,
							bitmap,
							0.003,
							0,
							15.0,
							0.05,
							left,
							0,
							LABEL_COLOR,
							1.5,
							face_viewer_horiz,
							0,
							0.0,
							GLUT_BITMAP_HELVETICA_12,
							1 // diagonal off
						);
	LabelAttributeObj terminalLabelAttributes2;
	terminalLabelAttributes2 = labelAttributeObjAlloc(); 
	labelAttributeObjInit(
							terminalLabelAttributes2,
							stroke,
							0.003,
							1,
							15.0,
							0.05,
							left,
							0,
							LABEL_COLOR,
							1.5,
							face_viewer_radial,
							1,
							0.0,
							GLUT_STROKE_ROMAN,
							0
						);

LabelAttributeObj terminalLabelAttributes3;
	terminalLabelAttributes3 = labelAttributeObjAlloc(); //horizontal bitmap terminals
	labelAttributeObjInit(
							terminalLabelAttributes3,
							stroke,
							0.003,
							0,
							30.0,
							0.05,
							center,
							0,
							LABEL_COLOR,
							1.5,
							face_viewer_horiz,
							0,
							0.0,
							GLUT_BITMAP_HELVETICA_12,
							0
						);

	LabelAttributeObj internalLabelAttributes1;
	internalLabelAttributes1 = labelAttributeObjAlloc(); //horizontal stroke internals
	labelAttributeObjInit(
							internalLabelAttributes1,
							stroke,
							0.01,
							0,							// do not attenuate
							15.0,
							0.05,
							left,
							0,
							LABEL_COLOR,
							1.5,
							face_viewer_horiz,
							0,
							0.0,
							GLUT_BITMAP_HELVETICA_12,
							1							// add diagonal lines
						);
// NB!! Need to initialize branch att objects with copy constructor!!
	float ctlpointParms[4] = {0.5,0.8,0.8,0.5};
	BranchAttributeObj branchAttributesTree, branchAttributesSubtree, branchAttributesSubtree2;
	branchAttributesTree = branchAttributeObjAlloc();
	branchAttributesSubtree = branchAttributeObjAlloc();
	branchAttributeObjInit(
						branchAttributesTree,
						tube,
						nurbs,
						1.0,
						TREE_COLOR,
						0,
						1,
						15.0,
						0.05,
						ctlpointParms
						);
	branchAttributeObjInit(
						branchAttributesSubtree,
						line,
						straight,
						1.0,
						SUBTREE_COLOR,
						1,  // corners broken, setup doesn't recognize this attribute, but global gCorners littering everywhere
						1,
						15.0,
						0.05,
						ctlpointParms
						);
	branchAttributesSubtree2 = branchAttributeObjAlloc();
	branchAttributeObjInit(
						branchAttributesSubtree2,
						tube,
						nurbs,
						1.0,
						TREE_COLOR,
						0,
						1,
						45.0,
						0.25,
						ctlpointParms
						);


// Here is where we finally get down to the business of building the oglTree

	theOglTree = oglTreeAlloc();
	gRootOglTree = theOglTree;
#define HEIGHT_RADIUS_RATIO 1.5


	// Insert any internal node names from MRCA lists if those are present (must be done prior to setting up compact nodes)
	internalNamesFromMRCACommand(root);
	// Just identify the nodes that will be targetted by the hierarchical layout
	setupCompactNodes( root, MIN_CLADESIZE, MAX_CLADESIZE, COMPACT_NUM_ITERATIONS, intNameAlgorithm);


	oglTreeInit(theOglTree, root, NULL, gLayoutChoice, CIRCLE_RADIUS, 1.5*CIRCLE_RADIUS, gCircleFraction,branchAttributesTree, terminalLabelAttributes3,internalLabelAttributes1, TREE_FREE, 0.0,1,0,0);


	// If we will display using a visual hierarchy: clades, subclades, etc.
	if (gHierarchical)
		{
		theOglTree->numSubclades = setupSubcladesFromCompactNodes(theOglTree, theOglTree->root, branchAttributesSubtree2, terminalLabelAttributes3, internalLabelAttributes1);
		// OOPS! I'm passing the same one and only attributes object to every subtree, but these have to have different scaleSizes, for example
		subcladeMarkInit(theOglTree->root);
		//initMarkedAnc(theOglTree->root,NULL); not needed


		setupSubcladesSubordinate(theOglTree, theOglTree->root, branchAttributesSubtree, terminalLabelAttributes2, internalLabelAttributes1); // these are small clades along the lines to the large clades.


		}
//	oglTreeLayout(theOglTree); // do this AFTER subclades have been setup
	gBaseTreeRoot=gTreeRoot=root; // the one global needed for display routine and its cylinder drawing routine 

	// First do the actual coordinate calculations for all the layouts
	setHierarchicalCoordinateLayoutScheme(theOglTree);
	// then do some initializations of the visual display layouts, which must be done after the coords have been calculated.
	setHierarchicalDisplayLayoutScheme(theOglTree,layoutArray1,faceArray1,MAX_SCENE_TREE_DEPTH,0);
	
	tree_openGL(argc,sargv,theOglTree); // this is all the openGL code 
	return;
}



// ******************************************************************************************************************************
// ******************************************************************************************************************************



static float setSubtreeRadius(long nTree, long nSubtree, float rTree, float minRFraction, float maxRFraction, int nMin, int nMax)
	{
// nTree,nSubtree = numdescEffective of tree and subtree
// rTree = radius of original tree
// minRFraction, maxRFraction = minimum and maximum allowed size of subtree as a fraction of original tree
// nMin, nMax = between these numbers, the size of the subtree will linearly change depending on how many taxa,
//		but above or below these numbers the radius of the subtree will be pinned to its min/max values, depending on 
//		minRFraction, maxRFraction

// Not currently using nTree in the calculation, but it might make sense to have subtrees be smaller when there are lots of subtrees

	float minRadius, maxRadius, r;
	minRadius = rTree*minRFraction;
	maxRadius = rTree*maxRFraction;

	r = minRadius + (nSubtree-nMin)*(maxRadius-minRadius)/(nMax-nMin);
	
	if (nSubtree > nMax) return maxRadius;
	if (nSubtree < nMin ) return minRadius;
	return r;
	}
/***********************************************************/

OglTree oglTreeAlloc(void)
	{
    OglTree t;
	t = (OglTree)malloc(sizeof(struct oglTreeStruct));
	return t;
	}

/*

Initialize a tree to have a size of 'radius'. This will be scaled later, depending on many factors.


The initial layout is either the area of a circle (which will later be 'stretched' to fit the surface of a sphere or
hemisphere), or the circumference of a circle (an arc), both of which can be handled with the same 'area' data structure.

All layouts proceed with a downpass and then an uppass. In the downpass, we start at root and 'split' the 'area' of the
layout recursively giving size to the offspring that is proportional to the number of taxa in the offspring clades. 
This procedure generates an 'area' data structure for every node in the tree. This structure has polar coordinates for
r1, r2, theta1, theta2. Also, a default value for the center of the area is constructed, which will remain valid for
terminals. The center will be the point in space where nodes will be laid out.

In the uppass, the centers of internal nodes are corrected in various ways. Usually, this is done to weight the position
depending on the positions of its descendant nodes, typically the mean value of the positions of the descendants. This
is done with setupCenters or setupArcCenters functions.

At this point different layouts start to handle setting up r and z values differently...

*/

void oglTreeInit(OglTree t, node root, node parentNode, enum LayoutChoice layout, float radius, float height, float circleFraction, BranchAttributeObj branchAttributes, LabelAttributeObj terminalLabelAttributes,LabelAttributeObj internalLabelAttributes,enum FaceStyle faceStyle, float treeXRotate, short displayTerminalLabels, short displayInternalLabels, short doAttenuateBoundingShape)

	// get rid of int branchStyle, terminalScheme, ..deprecated

	// layout choice is now overridden by setupHierarchicalCoords...()

	{
	extern double gZRange;
	double numturns;
	area Ap, A0;
	double x,y,theta;
	double radiusOfOriginalCircle;
        float max_radius,tree_radius,tree_height;

	t->anc = NULL;
	t->firstdesc=NULL;
	t->sib=NULL;
    t->root=root;
	//t->root->label = rootStr; // This is just some default string. For base tree, this is not currently initialized...
	t->parentNode=parentNode;
	t->numChildren=0;
	t->circleFraction = circleFraction;
	t->treeXRotate=treeXRotate;
	t->faceStyle=faceStyle;
	t->terminalLabelAttributes = terminalLabelAttributes;
	t->internalLabelAttributes = internalLabelAttributes;
	t->branchAttributes = branchAttributes;
	t->layout=layout;
	t->defaultLayout=layout;
	//t->parentTree=parent;
	t->numSubclades = 0;
	t->height = height;
	t->outerRadius=radius; // just a default for non circle layouts; the nondefault is set in oglTreeLayout()
	t->petioleLength=radius; // just a default
	t->leafBaseLength=0.0; // just a default
	t->displayTerminalLabels=displayTerminalLabels;
	t->displayInternalLabels=displayInternalLabels;	
	t->doAttenuateBoundingShape=doAttenuateBoundingShape;
	t->animating = 0;	// not animating this tree by default
	t->upDirection = 0.0;
	numdesc(root);			// sets up number of leaves descended from any node
	numdescEffective(root); // sets up number of leaves or collapsed nodes descended from any node (<=
	t->nLeaves = root->numdesc;
	t->numdescEffective = root->numdescEffective;
	
	t->maxLabelLength = maxLabelLengthGlut(root,&(t->maxLabel),t->terminalLabelAttributes->font);
	
		
	maxorder(root); 	// this is the only time we need to make sure to use the old maxorder...?

	extern float gTreeAttenuate, gLabelAttenuate;
	t->radius = radius;	// overrides previous


	return;
	}
	
static void oglTreeLayout(OglTree t) 
	{
	extern double gZRange;
	double numturns;
	area A0;
	double x,y,theta;
	double radiusOfOriginalCircle;
	float max_radius,tree_radius,tree_height;
	float radius;
	node root;
	root = t -> root;
	radius = t->radius;
	
	maxorderEffective(t->root); // !!!!!!!!!!!!!! EXPERIMENTAL !!!!!!!
	
	setRadius(t->root, t->root, gRootRadius, gLeafRadius); // these are the tube radii, not the tree's radius

#define SPACE_AT_TREE_MARGINS 0.98
//	switch (t->layout)
	switch (t->coordLayout)
	    {
	    case goblet:;
	    case cone:; 
			{
			/*
			 */
			gRadiusOfOriginalCircle=radiusOfOriginalCircle=radius;
			numturns=SPACE_AT_TREE_MARGINS*t->circleFraction;  // leaves a little space around the two sides
			gZRange=t->radius;
			A0=areaNew(radius,radius,0.0,numturns*TWO_PI); // for arcs (spiral and circ), set inner and outer radii equal
			root->data=A0;
			treeSplit(root);
			setupArcCenters(root,&theta);
			assignR(root,radius);
			setupArcZ(root);
			gViewerZ=radius;  // place the viewpoint at a distance equal to the radius
			//t->height=radius; // override whatever might have been provided as input
			break;
			}
	    case circle:
			{

			/*
			 The tree is a circle with radius of radius, placed at the origin with its axis as the Z axis.
			 The viewpoint is at +Z = radius.
			 For half circles and the like, the root is at (x,y)=(0,0), and the tree extends in the +x direction spreading equally in the +y and -y axes.
			 */
			gRadiusOfOriginalCircle=radiusOfOriginalCircle=radius;
			numturns=SPACE_AT_TREE_MARGINS*t->circleFraction;  // leaves a little space around the two sides
			float start, end;
			start = (1-numturns)/2.0;
			end = 1-(1-numturns)/2.0;
			gZRange=0.0;
//			A0=areaNew(radius,radius,0.0,numturns*TWO_PI); // for arcs (spiral and circ), set inner and outer radii equal
//			A0=areaNew(radius,radius,start*TWO_PI-PI/2,end*TWO_PI-PI/2); // for arcs (spiral and circ), set inner and outer radii equal
				// Subtraction of PI/2 is centering the tree's leaves about the +x-axis, with the root at x=0,y=0
			A0=areaNew(radius,radius,start*TWO_PI-PI,end*TWO_PI-PI); // for arcs (spiral and circ), set inner and outer radii equal
				// Subtraction of PI/2 is centering the tree's leaves about the +x-axis, with the root at x=0,y=0
			root->data=A0;
			treeSplit(root);
			setupArcCenters(root,&theta);
			assignR(root,radius);
			setupArcZ(root);
            adjustLeafRForEllipse(root, ECC_FACTOR);
            gViewerZ=radius;  // place the viewpoint at a distance equal to the radius
			t->height=0; // override whatever might have been provided as input

			if (t->terminalLabelAttributes->textStyle == stroke && t->terminalLabelAttributes->orientation == face_viewer_radial) 
				{
				t->terminalLabelAttributes->scaleSize = calcLabelScaleCircle(t->numdescEffective, t->radius, t->circleFraction);
				t->outerRadius = t->terminalLabelAttributes->strokeTerminalOffset + t->radius + 1.1*t->terminalLabelAttributes->scaleSize*t->maxLabelLength;
				}
			t->petioleLength = t->outerRadius*0.5;
			t->leafBaseLength= t->outerRadius*0.2;

			break;
			}
	    case spiral:
			{
			gRadiusOfOriginalCircle=radiusOfOriginalCircle=radius;
			numturns=1.5*t->circleFraction; // default will spiral around another half circle
			gZRange=radius/4;
			A0=areaNew(radius,radius,0.0,numturns*TWO_PI); 
			root->data=A0;
			treeSplit(root);

			setupArcCenters(root,&theta);
			setupArcZ(root);
			assignR(root,radius);
			gViewerZ=radius;
			//t->height=gZRange;  // not sure about the height here !!!! CHECK IF YOU USE SPIRAL AGAIN !!!!
			break;
			}
	    case hemisphere:  // change to use CIRCLE_RADIUS
			// the tree is centered on the root node within its bounding box
			{
				/*
				 The tree is a 3D tree with radius and height of HEMISPHERE_RADIUS, placed at the origin.
				 The viewpoint is at +Z = HEMISPHERE_RADIUS.
				 The circle is originally set to 1.0, but the coneToHemiSphere code uses the z-values set up in assignZ 
				 (which range from 0 to HEMISPHERE_RADIUS) to warp the cone out to this larger radius.
				 */

			A0=areaNew(0.0,1.0,0.0,TWO_PI); // passing a radius of 1.0 into this function; scale back to radius later
			root->data=A0;
			treeSplit(root);
			setupCenters(root,&x,&y);	
			assignZ(root,0, radius);		// this radius will get used in the next call
			coneToHemiSphere(root,1.0); // if you want to do sphere or hemisphere layout use this; also use 1.0 radius here
			gViewerZ=radius;
			t->height=radius;		// override input
			break;
			}
	    case solid_cone: 
				// the tree is centered at the vertical midpoint between root and most distant terminal, EXCEPT when subtrees are present
				// in which case, the midpoint has not been correctly calculated yet, and thus initial viewer and rotations may be about
				// an arbitrary fixed point inside the tree along the z-axis.
                /*
                 This has two modes: cone mode and cone mode modified with FDP (if -I is set to a value > 0)
                 In cone mode the tree is a 3D tree with radius of CIRCLE_RADIUS and height of CIRCLE_RADIUS, placed at the origin.
                 The viewpoint is at 'height'.
                 In FDP mode the tree is 3D with radius and height of CIRCLE_RADIUS.
                 */
			{
			A0=areaNew(0.0,radius,0.0,TWO_PI);
			root->data=A0;
			treeSplit(root);
			setupCenters(root,&x,&y);
			//t->height = t->radius * heightRadiusRatio;
			assignZ(root,-t->height/2.0,t->height/2.0);
			if (gIterFDP > 0)
			(void)FDP(root,gIterFDP,radius,radius/20.0,gAttractFactor);
			gViewerZ=t->height;  // not positioning initial tree right yet...
			break;
			}
	    }
		
	setup_cylinders_trig(t, root);
//	treeStyleInit(root, treeColor, branchStyle,REPLACE_UNDERSCORES);  CAREFUL !!!!!!!!!!!!!!!!!!!!

	return;
    }




/***********************************************************************************/

static void internalNamesFromMRCACommand(node root) 

// Now we just use the mrca list to insert internal names into tree structure for later use


// We need to watch out for cases in which the mrca list induces a huge clade, simply because there is some weird
// slight departure from monophyly on the tree. I do this by checking a tolerance factor, Z. Let x be the number of
// leaves in the clade and X be the number of labels in the mrca list. If x/X > Z we do NOT make this a clade name
// and issue a warning instead.
// Hmm, well that's fine when we have a good sample of leaves from the clade in the mrca list, but suppose we have just two?
// Then we'll frequently bork on such commands, no good. So, I really should reserve this option for cases where we know
// there is a nearly complete sample of the e.g., genus names. DISABLE FOR NOW


	{
	long i,x,X;
	float Z = 2.0;
	Entry p,e;
	char * mrcaName;
	slist taxList;
	node n, n1;
	if (!mrcaHash)
		return; // maybe we didn't set any up in parser
	else
		printf("Number of internal names stored:%li\n",mrcaHash->numElements);
	printf("Warning: when setting up mrca clades, we are doing some nontrivial filtering of clade definitions (see code!)\n");
	for (i=0;i<mrcaHash->numElements;i++)
			{
			p=hashGetKthEntry(mrcaHash,i);
			mrcaName = p->key; 
			taxList  = (slist)(p->val);
			
			n=mrca(root,taxList, &X);
			if (n)
				n->label = DupStr(mrcaName);
#if 0
			if (n)
				{
				x = numdescNoSave(n);
				if (x/X < Z)
					{
					n->label = DupStr(mrcaName);
					printf("Internal name %s was set up with %li leaves from %li matching labels\n",mrcaName,x,X);
					}
				else
					printf("Internal name %s was NOT set up with %li leaves from %li matching labels\n",mrcaName,x,X);
				}
			else
				printf("Warning: Internal name %s could not be assigned to internal node\n",mrcaName);
#endif


			}
	}


 
/*
Notes on subclades.

	0. Definitions.
		-"Complete tree" is the tree with all leaves; some internal nodes are set to be compact nodes.
		-"Subtree" is a tree with a compact node as its root, including all of its leaves (and other compact nodes).
		-A subtree has a "parent tree", which is the smallest subtree that contains it, this induces a tree of subtrees.
		-A subtree has a "parent node", which is the compact node on the parent tree, corresponding to the root on the subtree.
		-The "root subtree" is the deepest parent tree, and it contains the complete tree.

	1. A subtrees tree data structure is separate from those in other subtrees, but may be redundant. It goes from the subtree's root to all leaves, not just the descendant compact nodes!!! This seems wasteful but it eases some of the bookkeeping about parent nodes of subtrees, etc.
	2. The layout calculations, on the other hand, go from the root of a subtree to its leaves OR compact nodes, which are considered leaves for the purposes of layout.
	3. A subtree's parentNode points to the node on its parent tree, not the node on the root subtree...otherwise
	some of the nodes on the root tree may not have been laid out by layout algorithms yet. 
	4. parentNode is setup in oglTreeInit by passing the correct node.
	
	Yes, we might consider using a single underlying tree structure, rather than copying bits and pieces of it, but that's ok for now.
	

*/

static long setupSubcladesFromCompactNodes(OglTree theOglTree, node a, BranchAttributeObj branchAttributes, LabelAttributeObj terminalLabelAttributesTemplate, LabelAttributeObj internalLabelAttributes)
	{
	OglTree newOglTree, parentOglTree, t;
	LabelAttributeObj terminalLabelAttributes;
	node child,n1;
	float newRadius;
	long i, sum=0;
	int forceRedo=0;
	char *name;
	parentOglTree = theOglTree; // this will be passed to children by default

	if (a -> isCompactNode) // skip the root: first, we're doing subtrees, plus, root->anc is NULL
		{
			// need a local attributes obj for each subtree, get from template
			terminalLabelAttributes = labelAttributeObjInitByCopy(terminalLabelAttributesTemplate); //radial stroke terminals

			// Currently we use an existing internal label if it's there; otherwise make up a fancy one...
			if (!a->label)
				internalNameCreator2(a);
			n1 = copyTree(a);

			n1->anc=NULL; // necessary if copying a subtree -- should force this in copyTree code...
			n1->isCompactNode=0; // force the new root of this tree NOT to be called a compact node...

			newRadius = setSubtreeRadius(gRootOglTree->root->numdescEffective, a->numdescEffective, gRootOglTree->radius, MIN_SUBTREE_FRACTION, MAX_SUBTREE_FRACTION, MIN_CLADESIZE, MAX_CLADESIZE); // enforce calculation of subtree sizes relative to the entire tree, rather than immediate parent tree

			newOglTree = oglTreeAlloc();
			oglTreeInit(newOglTree,n1, a, circle, newRadius, 2*newRadius, CIRCLE_FRACTION,branchAttributes,terminalLabelAttributes,internalLabelAttributes, TREE_FACES_VIEWER,0,1,1,0);

			addChild(theOglTree,newOglTree); // make the scene graph tree
//			oglTreeLayout(newOglTree); // do this AFTER any subclades have been setup...do I need to do this after recursing in below?
// PREVIOUS NOW HANDLED BY setupHierarchicalCoords...			
			// careful here; i may have to copy isCompactNode flag from somewhere...YES, MODIFY COPYTREE() TO DO THIS COPYING, AND POSSIBLY OTHER FIELDS IN NODE STRUCT
			printf("Set up regular subtree %s\n",n1->label);
			
			sum = 1;
			parentOglTree = newOglTree;		// now this new tree becomes the parent of any children found in the recursion
			a = n1; // continue traversing on the new subtree, not the parent tree; see next statement
			forceRedo=1; // deprecated
		}
	child=a->firstdesc;
	SIBLOOP(child)
		sum += setupSubcladesFromCompactNodes(parentOglTree, child, branchAttributes, terminalLabelAttributesTemplate, internalLabelAttributes);


///// Probably want to do subcladeMarkInit and initMarkedAnc here too for tertiary layouts...and see question above...

	return sum;
	}



static long setupSubcladesSubordinate(OglTree theOglTree, node a, BranchAttributeObj branchAttributes, LabelAttributeObj terminalLabelAttributesTemplate, LabelAttributeObj internalLabelAttributes)
	{
	OglTree newOglTree, parentOglTree;
	LabelAttributeObj terminalLabelAttributes;
	node child,n1;
	float newRadius;
	long i, sum=0;;
	char *name;
	parentOglTree = theOglTree; // this will be passed to children by default


	if (a->isCompactNode) return; // MUST REVISIT THIS! FOR 3+ LEVEL LAYOUTS THIS NEEDS WORK; OK FOR TWO?
		// All hell breaks loose without the previous, as we recurse inside subclades and try to make subord subclades
		// that have the root as its parent, while also being descended from the subclade...
	if (!a -> isCompactNode  && !a->marked && a->numdesc > 1)
		{
			// need a local attributes obj for each subtree, get from template
			terminalLabelAttributes = labelAttributeObjInitByCopy(terminalLabelAttributesTemplate); //radial stroke terminals

			a->isCompactNode = 1 ; // ?????????? Do I really want to do this????

			if (!a->label)
				internalNameCreator2(a);

			n1 = copyTree(a);
			n1->anc=NULL; // necessary if copying a subtree -- should force this in copyTree code...
			n1->isCompactNode=0; // force the new root of this tree NOT to be called a compact node...

			newRadius = setSubtreeRadius(theOglTree->root->numdescEffective, a->numdescEffective, gRootOglTree->radius, MIN_SUBTREE_FRACTION, MAX_SUBTREE_FRACTION, MIN_CLADESIZE, MAX_CLADESIZE);
			newOglTree = oglTreeAlloc();
			oglTreeInit(newOglTree,n1, a, circle, newRadius, 2*newRadius, CIRCLE_FRACTION,branchAttributes,terminalLabelAttributes,internalLabelAttributes, TREE_FACES_VIEWER,0,1,0,1);
			addChild(theOglTree,newOglTree); // make the scene graph tree
//			oglTreeLayout(newOglTree); // do this AFTER any subclades have been setup'
// PREVIOUS NOW HANDLED BY setupHierarchicalCoords...			
			
			// careful here; i may have to copy isCompactNode flag from somewhere...YES, MODIFY COPYTREE() TO DO THIS COPYING, AND POSSIBLY OTHER FIELDS IN NODE STRUCT
			
			printf("Set up subordinate subtree %s\n",n1->label);
			sum = 1;
			parentOglTree = newOglTree;		// now this new tree becomes the parent of any children found in the recursion
			//a = n1; // continue traversing on the new subtree, not the parent tree; see next statement
			return;  // no, unlike the usual situation, no need to recurse anymore here...
		}
	child=a->firstdesc;
	SIBLOOP(child)
		sum += setupSubcladesSubordinate(parentOglTree, child, branchAttributes, terminalLabelAttributesTemplate, internalLabelAttributes);


///// Probably want to do subcladeMarkInit and initMarkedAnc here too for tertiary layouts

	return sum;
	}





static void setupCompactNodes(node root, int minsize, int maxsize, int numIterations, enum IntNameAlgo intNameAlgorithm)
	/*
	First step in creating a hierarchical is to identify the nodes
	that will form the roots of these subclades. These are called compact nodes.
	
	To do that, we iteratively construct levels of subclades, starting with the most terminal level,
	adjusting the effective number of leaves after these terminal clades have been "collapsed" and then
	proceeding deeper in the tree.
	
	Notice this routine acts on the raw original phylogenetic tree data structure, prior to init of OglTrees
	*/

	{
	int numdescEffectivePrev = -1,ndEffective, count=1, i;
	
	numdescEffective(root); // just to make sure...


	switch (intNameAlgorithm)
		{
		case USER_ONLY:
			printf("Setting up hierarchical layout from user supplied internal node names only\n");
			setupCompactNodesHelper(root, minsize, maxsize,intNameAlgorithm);
			numdescEffective(root); // just to make sure...
			break;
		case TREE_ONLY:
			printf("Setting up hierarchical layout from tree only with %i levels\n",numIterations);
			for (i=1;i<=numIterations;i++)
				{
				setupCompactNodesHelper(root, minsize, maxsize,intNameAlgorithm);
				numdescEffective(root);
				ndEffective =  root -> numdescEffective;
				//printf("Compact node iteration %i: numdescEffective at root = %i\n",count++, ndEffective);
				if (ndEffective == numdescEffectivePrev)
					break;
				else
					numdescEffectivePrev = ndEffective;
				
				}
			break;
		case BOTH:
			printf("Setting up hierarchical layout with user supplied internal node names and from tree, with %i levels\n",numIterations); 			
			setupCompactNodesHelper(root, minsize, maxsize,USER_ONLY);
			numdescEffective(root); // just to make sure...
			for (i=1;i<=numIterations;i++)
				{
				setupCompactNodesHelper(root, minsize, maxsize,TREE_ONLY);
				numdescEffective(root);
				ndEffective =  root -> numdescEffective;
				//printf("Compact node iteration %i: numdescEffective at root = %i\n",count++, ndEffective);
				if (ndEffective == numdescEffectivePrev)
					break;
				else
					numdescEffectivePrev = ndEffective;
				
				}
			break;
		
		}
	}

static void setupCompactNodesHelper(node a, int minsize, int maxsize, enum IntNameAlgo intNameAlgorithm)
	// set compact node flag for clades that are >= minsize and <=maxsize; OR if an internal name is present
	// numdescEffective must be set up first.
	{
	node child,n1;
	float newRadius;
	long i, sum=0;
	//if (a->isCompactNode)
		//return;
	child=a->firstdesc;
	SIBLOOP(child)
		setupCompactNodesHelper(child, minsize, maxsize, intNameAlgorithm);
		

	//if (!isRoot(a) && a->numdescEffective >= minsize && a->numdescEffective <=maxsize && a->anc->numdescEffective > maxsize) 
	if (intNameAlgorithm==TREE_ONLY && !isRoot(a) && a->numdescEffective >= minsize && a->numdescEffective <=maxsize && a->anc->numdescEffective > maxsize) 
			// skip the root: first, we're doing subtrees, plus, root->anc is NULL
			{
			a->isCompactNode=1;
	//printf ("Doing a compact node at a node with %li leaves and %li effective leaves, anc eff leaves = %li\n", a->numdesc, a->numdescEffective, a->anc->numdescEffective);
			}
	if ( intNameAlgorithm==USER_ONLY && !isRoot(a) && !isTip(a) && a->label) 
			{
			a->isCompactNode=1;
			}

///// Probably want to do subcladeMarkInit and initMarkedAnc here too for tertiary layouts

	return;
	}







// Determine the needed label scale to fit stroke font labels on circle layout
// THIS IS NOW SIMPLY RETURNING A CONSTANT CHOSEN FOR ESTHETICS.

static float calcLabelScaleCircle(long n, float radius, float circleFraction)
	{
#if 0
	if (n<10) n = 20; // hack to prevent the labels getting too large when n << 10;


	float w; // = TWO_PI * radius * circleFraction / n;  // ogl modeling units vertical space available per label 
	w = 0.60;
	const float glutHpos = 119.05; // max hight units of a glut Roman stroke font above line and ... below
	const float glutHneg = 33.33;
	float glutH = glutHpos + glutHneg;
#endif	
	return 0.0015; // this is just a constant controlling size of labels
	
	//return w / glutH;
	
	}



float maxLabelLengthGlut(node n, char **maxLabel, void * font)  // getlength of longest label among leaves of this clade, where length is measured by GLUT function call; also return the label itself (deprecated now?)
	{
	node child;
	char *bestMaxLabel;
	float max=0, len;
// #define TRUNC_LABEL_N 25
    char trunc[TRUNC_LABEL_N+4];
	
	
	if (isTip(n))
		{
		*maxLabel = n->label;
        //strncpy(&trunc[0],n->label,TRUNC_LABEL_N);
        //trunc[TRUNC_LABEL_N]='\0';
		
		truncateLabel(n->label,trunc,TRUNC_LABEL_N);
		
//		return glutStrokeLength(font,(unsigned char*)n->label);
		return glutStrokeLength(font,(unsigned char*)&trunc[0]);
		}

	else if (!isRoot(n) && n->isCompactNode)
		{
		if (n->label ) // a root node might be compact but it is never a leaf in a rendered tree
			{
			*maxLabel = n->label;
			return glutStrokeLength(font,(unsigned char*)n->label);
			}
		else
			{
			*maxLabel = "";
			return 0;
			}
		}

	else
		{
		child=n->firstdesc;
		SIBLOOP(child) 
			{
			len=maxLabelLengthGlut(child, maxLabel, font);
			if (len > max)
				{
				max = len;
				bestMaxLabel=*maxLabel;
				}
				
			}
		*maxLabel=bestMaxLabel;
		return max;
		}
	
	}




/***********************************************************************************/


// not up to date
void printUsage(void)
{

		printf("Usage: ceiba -f datafile  [-d] [-t S][-I N] [-x S] [-max_images N] [-font S] [-compress_images] [-h] [-v]\n");
		printf("\t-f\tInput nexus formatted file name (required)\n");
		printf("\t-I\tSpecify number of iterations, N, in force-directed layout (default=25)\n");
		printf("\t-h\tPrint this message\n");
		printf("\t-v\tPrint version\n");
		printf("\t-d\tTurn off any hierarchical display (best for trees < 100 taxa)\n");
		printf("\t-compress_images\tUse texture compression (recommended when many large image files are used)\n");
		printf("\t-t\tUse image/object files of type specified by S, where S is one of {ppm, stl, xyz}\n");
		printf("\t-x\tSet hierarchical grouping algorithm to S, where S is one of {user_only, tree_only, both}\n");
		printf("\t-font\tSet font to S, where S is a valid font found in directory /Library/Fonts\n\t\t(quote string if it contains spaces, such as \"Arial Narrow\")\n");
		printf("\t-max_images N\tSet maximum number of images, N, to display at node (default=1)\n");
		
		printf("\n\nInteractive Commands\n");
		printf("\tMouse motion:\n");
		printf("\t\t(Left)Click and drag left-right rotates around z axis; up-down rotates around x axis.\n");
		printf("\t\t(Left)Shift-click and drag left-right moves tree along x axis; up-down zooms in and out.\n");
		printf("\t\tDouble click on a clade or image/object to animate rotations to front and center\n");
		printf("\t\tSingle click on a clade to highlight its tree and bring to front\n");
		
		printf("\n\nKeyboard Shortcuts\n");
		printf("\tESC\t\tQuit program\n");
		printf("\tf\t\tToggle full screen mode\n");
		printf("\tl/L\t\tDecreases/increases the size of horizontal taxon labels\n");
		printf("\tw/W\t\tDecreases/increases the size of any image objects\n");
		printf("\tc/C\t\tDecreases/increases the width of branches\n");
		printf("\tx/X,y/Y,z/Z\tMove the tree in these six possible directions\n");
		printf("\to/O\t\tTurn off/on rotation of tree animation\n");
		printf("\t[/]\t\tDecreases/increases the speed of tree rotation animation\n");
		
		printf("\n\nSearch for taxa\n\tRight click mouse and select taxon name (2-button mouse required)\n");

		
		
}
