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

#define SQR(x) ((x)*(x))

static void initLeafMenuHelperRecurse(node n, OglTree T);
static void initLeafMenuHelper(node n, OglTree T);
typedef struct node_level_map_struct { node n; OglTree t; } NodeLevelMap;
NodeLevelMap * gNodeLevelMapArray;
long gix2;

void initLeafMenu(OglTree T)
// traverse the scene tree and set up a leaf list map associating leaves with the trees they belong to
	{
	long nLeaves = T->nLeaves;
	gix2=0;
	gNodeLevelMapArray = (NodeLevelMap *)malloc(nLeaves*sizeof(NodeLevelMap));

	initLeafMenuHelper(T->root, T);

	return ;
	}
static void initLeafMenuHelper(node n, OglTree T)
	{
	OglTree child;
	initLeafMenuHelperRecurse(n,T);
	child = T->firstdesc;
	SIBLOOP(child)
		{
		initLeafMenuHelper(child->root, child);
		}
	}
	
static void initLeafMenuHelperRecurse(node n, OglTree T)
{
node child;
if (n->isCompactNode) return;
if (isTip(n))
	{
	gNodeLevelMapArray[gix2].n = n;
	gNodeLevelMapArray[gix2].t = T;
	++gix2;
	}
child = n->firstdesc;
SIBLOOP(child)
        initLeafMenuHelperRecurse(child,T);
}

int nodeLabelCompar2(const void *elem1, const void * elem2)

// sort comparison function: alphabetical sort, skipping over first non-alpha characters in string
// and if the string is ALL nonalpha, it gets sorted to end of list

{
NodeLevelMap *n1,*n2;
char s1[256],s2[256],*S1, *S2;
n1=(NodeLevelMap*)elem1; // ugh, first cast the void pointer for the comparator, then deref to get the node
n2=(NodeLevelMap*)elem2;
strcpy(s1,n1->n->label);
strcpy(s2,n2->n->label);
S1=firstalpha(s1);
S2=firstalpha(s2); // move into string until we get to first alphabet char
if (!S1) return 1;
if (!S2) return -1; // if one of the strings has no alpha chars, this will insure it gets sorted last?
strtoupper(S1);
strtoupper(S2);
return strcmp(S1,S2);
}

	
	
	
void setupCascadeMenu(void) //  move the caller for this out of initializeGLUT...??

/*
Sets up a menu and cascading submenus containing all the terminal taxon names in the tree.
The first menu is a list of submenus sorted by the first letter of the taxon name, A-Z. 
(Prepending nonalpahbetic characters are skipped in the sort, and any name that is entirely
nonalphabetic is skipped--not put in list). Submenus are possibly broken into a third level if
there are more than MAX_MENU_ENTRIES. The reason is execution speed. GLUT really bogs down with
menus that are much longer than the number of lines vertically on the screen.
*/

{
#define MAXSUBMENUS 5000
extern node gTreeRoot;
int firsti,lasti,savessx,submenu[26],menu,i,k,id=0,j,jx=0,lbin[26],ssx=0,subsubmenuNum[26],remaining,subsubmenu[MAXSUBMENUS];
char s[128], *s1;
char letter='A',letterA='A',*firstc,*firstTaxon,*lastTaxon;
long numTaxa;
extern nodeArray gNodeArray;
numTaxa=numdesc(gTreeRoot);
gNodeArray=newTipNodeArray(gTreeRoot);
qsort(gNodeArray,numTaxa,sizeof(node),nodeLabelCompar);
qsort(gNodeLevelMapArray,numTaxa, sizeof(NodeLevelMap),nodeLabelCompar2);

#define MAX_MENU_ENTRIES 40  // perhaps should get this adaptively from screen height
for (i=0;i<26;i++) // 26 bins for the letters A-Z 
	lbin[i]=0;
for (i=0;i<numTaxa;i++)
	{
	s1=(gNodeLevelMapArray[i].n)->label;

//printf("%s\n",s1);
	firstc = firstalpha(s1);
	if (firstc)
		++lbin[toupper(*firstc)-letterA];
	}
for (i=0;i<26;i++)
	{
	if (lbin[i]<=MAX_MENU_ENTRIES)
		subsubmenuNum[i]=0;
	else
		subsubmenuNum[i]= (float)lbin[i]/MAX_MENU_ENTRIES;
	}

for (i=0;i<26;i++)
	{
	savessx=ssx;
	firsti=jx;
	for (j=1;j<=subsubmenuNum[i];j++)
		{
		subsubmenu[ssx++]=glutCreateMenu(handleMenuClickCascade);
		for (k=1;k<=MAX_MENU_ENTRIES;k++)
			{
			s1=(gNodeLevelMapArray[jx].n)->label;
			glutAddMenuEntry(s1,jx++);
			}
		}
	submenu[i]=glutCreateMenu(handleMenuClickCascade);
	for (j=1;j<=subsubmenuNum[i];j++)
		{
		firstTaxon=(gNodeLevelMapArray[firsti].n)->label;
		lastTaxon =(gNodeLevelMapArray[firsti+MAX_MENU_ENTRIES-1].n)->label;
		sprintf(s,"%.8s...%.8s",firstTaxon,lastTaxon);
		glutAddSubMenu(s,subsubmenu[savessx++]);
		firsti+=MAX_MENU_ENTRIES;
		}
	remaining=lbin[i] - subsubmenuNum[i]*MAX_MENU_ENTRIES; // the entries not in a sub sub menu
	for (j=1;j<=remaining;j++)
		{
		if (jx >= numTaxa) goto out1;
		s1=(gNodeLevelMapArray[jx].n)->label;
		glutAddMenuEntry(s1,jx++);
		}
	}
out1:menu=glutCreateMenu(handleMenuClickCascade);
for (i=0;i<26;i++)
	{
	sprintf(s,"%c",letter++);
	glutAddSubMenu(s,submenu[i]);
	}
glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// ****  Animation code ****

/*
 The menu displays all leaf labels and a click starts an animation bringing that leaf into the foreground
 It does several things along the way:
	1. restores the tree to having its vertical axis toward the top of screen
	2. restores the position of the tree to x=0
	3. rotates tree so the desired taxon is in the foreground
	4. moves the tree vertically so the taxon is centered



*/
void handleMenuClickCascade(int k)
/*
When a name in the menu is clicked, retrieve the node and its containing tree.
Recurse rootward in the scenetree until we find a tree with a circle or solid layout.
Rotate in the usual animationg to make this circle centered and near eye.
Also, expand the descendant fans into circles so we can see exactly which circle has the taxon.
Finally, set gCurrOglSubtree to the original found subtree (now expanded to a circle) -- thus
the main display routine will write it twice to make it overlay other clutter.
TO DO: de-emphasize all other clades not in this descendant lineage.
*/

{
extern nodeArray gNodeArray;
extern OglTree gCurrOglSubtree;
extern int gNumExpandedFanlets;
extern node gCurrPickedNode;
struct PickableObject p;
initPickableObject(&p,LABEL,gNodeLevelMapArray[k].t, gNodeLevelMapArray[k].n,0.0);
node n = gNodeLevelMapArray[k].n;
OglTree T, t = gNodeLevelMapArray[k].t; 
T=t;
gCurrOglSubtree = t; // will overexpress this subtree for viewing
gCurrPickedNode = n;
// set the layout of all ancestor trees back to solid tree so they will be drawn as expanded circles, and tally number
while (t->layout != circle && t->layout != solid_cone && t->anc)
	{
	t->layout=circle;
	gNumExpandedFanlets++;
	t=t->anc;
	}


//animateRotationToNode(T, n, ROTATE_TO_NODE, 0,0,0); 
animateRotationToObject(&p); 
/*
if (t->layout == solid_cone )
	animateRotationToNode(t, n,0,0,0); 

if (t->layout == circle )
	animateRotationToNode(t, t->parentNode,0,0,0); // parent node has the coords we need rather than root node of t.
*/
printf ("Clicked on %s: %s\n",n->label, t->root->label);

}


//void animateRotationToNode(OglTree T, node n, enum FlagTypes rotateTo, float x, float y, float z) // move to node, but offset by xyz
void animateRotationToObject(struct PickableObject *p)
/*
On menu selection of double click, animate a rotation that brings either a leaf node or a fan into the foreground at a nice position.
Responding to a menu driven taxon name selection, with ROTATE_TO_NODE we move until the leaf's node position is centered on the screen and "close".
Responding to a double click on a visible fan, with ROTATE_TO_TREE we move until the center of the fan is centered on the screen and close.
Close is determined by 'finalEyeDistance', currently set to a fraction of the root tree's radius.

*/

{
extern float theta[3], theta0[3];
area A, Athis,Aanc;
int i;
extern double gRadiusOfOriginalCircle;
extern float xStep,yStep,thetaStep,zStep,theta0step;
extern int animSteps;
extern node gTreeRoot;
OglTree t;
float startTheta,endTheta,curTheta,startZ,endZ,startTheta0,endTheta0,diff0,diffTheta, radius;

float x,z;

OglTree T=p->tree;
float y=-p->height/2.0;
node n=p->nd;
enum ObjectType objType=p->objType;


// if the node is part of a subtree, find the marked ancestor node, and use its coordinates

float finalEyeDistance; // the distance from eye to tree or node after animation is over.
float deltaX=0.0, deltaY=0.0, angle;


switch (objType)
	{
	case LABEL: // rotate to make a label on any kind of tree front and center
		t=T;
		if (t)
		while (t->layout == circle)
			{
			A = (area)(n->data); // this will recurse through the leaf's ancestors.
			angle = A->theta_center - t->upDirection;  // what about petiole???
			deltaX += -sin(angle)*(t->outerRadius+t->petioleLength);
			deltaY += cos(angle)*(t->outerRadius+t->petioleLength); // sin and cos are backwards from intuition; remember the fans go 
			n = t->parentNode;
			//printf ("%f %f %f %f\n",toDegs(angle),deltaX, deltaY,t->upDirection);
		
			t=t->anc;
			}
		A=(area)(n->data); 
		finalEyeDistance = t->radius/10.0; // for example
		radius = A->r_center + finalEyeDistance;
		break;

	case IMAGE:
		switch (T->layout)
			{
			case circle:
				return;
			case solid_cone:
					A=(area)(n->data); 
					finalEyeDistance = T->radius/10.0; // for example
					//y=-0.2*T->outerRadius; // for taxon name, zoom to leaf node position 
					radius = A->r_center + finalEyeDistance;
			}
		break;

	case FAN: // rotate to make fan front and center
		if (T->layout == fanlet) return;
		if (T->anc)
			switch (T->anc->layout)
				{
				case solid_cone:
					A=(area)(T->parentNode->data);
					finalEyeDistance = T->anc->radius/10.0; // for example
					radius = A->r_center + finalEyeDistance;
					break;
				case circle:
					break;
				}
		break;
	}

#define ANIM_STEPS_FACTOR 3000 // bigger is smoother and slower
animSteps = ANIM_STEPS_FACTOR/gTreeRoot->numdescEffective + 1; // scale the number of animation steps to be fewer in large trees...

startTheta=theta[2];
diffTheta= - (90 + theta[2] +  toDegs(A->theta_center)  ); // took a lot of peering to get this right
	if ( diffTheta > 180 ) diffTheta -= 360; 
	if ( diffTheta < -180 ) diffTheta += 360; 

//printf("Initial theta = %f target theta = %f adjusted diff = %f\n",startTheta,endTheta,diffTheta);

thetaStep=diffTheta/animSteps;


extern OglTree gCurrOglTree;
zStep=(radius-gP->Z)/animSteps;

yStep=(A->z-gP->Y - y + deltaY)/animSteps;

xStep= (-gP->X + deltaX)/animSteps; // put the viewer back lined up with x=0 

startTheta0=theta[0];  // this is putting the tree back to its completely vertical original pos
endTheta0=theta0[0];
diff0 = endTheta0 - startTheta0; 
	if ( diff0 > 180 ) diff0 -= 360; 
	if ( diff0 < -180 ) diff0 += 360; 

theta0step = diff0/animSteps; 
    
glutIdleFunc(animateTwist);
}



// *************************

void setupMenu(void)
{
int numClades,menu,i;
extern Hash subclades;
Entry e;
if (!subclades) return;
menu=glutCreateMenu(handleMenuClicks);
glutAddMenuEntry("Root tree",0);  // 0th menu item is the root tree
numClades = subclades->numElements;
for (i=0;i<numClades;i++)
	{
	e= hashGetKthEntry(subclades,i);
	if (e)
		glutAddMenuEntry(e->key,i+1);// index is a code for which entry; 0 = root tree; 1...n are the subclades
	}
glutAttachMenu(GLUT_RIGHT_BUTTON);
}


void handleMenuClicks(int k)
{
extern Hash subclades;
extern node gTreeRoot, gBaseTreeRoot;
node subn;
Entry e;
if (!subclades) return;
if (k==0)
	gTreeRoot=gBaseTreeRoot;
else
	{
	e= hashGetKthEntry(subclades,k-1); // -1 to keep in register with my coding a few lines above...
	if (e)
		{
		subn = e->val;
		//setupTheTree(subn);
		gTreeRoot=subn;
		}
	}
plane_update();
glutPostRedisplay();
}

