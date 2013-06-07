/*** Force directed layout code **** */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mySmallTreeLib.h"
#include "tree_openGL.h"
//#include "layout.h"
#include <math.h>
#include <string.h>
#include "fdp.h"

static float distance(area A, area B);
static float f_attraction(float d);
static float f_repulsion(float d);
float gFDP_k,gFDP_k_sqr,gFDP_k1;

#define MIN(a,b) (((a)<(b))?(a):(b))

/*

I implemented a weighting scheme whereby subtrees at compact nodes receive repulsion in proportion
to their numdesc. I still do not like how little terminal branches off the main trunk have their cherries, etc., very close
in space. This happens I think because the whole thing expands, and then we renormalize, and relative to
the boundaries of the renormalized bounding object, these get squeezed relatively tightly together.

Experimented with doing z-axis moves on just these lateral branches, but gave up after modest effort. 
They shoot off to +- infinity.

Feb 1. 2013. I removed the weighting scheme, opting instead for a very increase in the attract_factor (default=5000)
in the main code. This provides more pleasant layouts when the number of effective leaves is ~25 - 35.

*/


/* Function FDP(...)

Force directed layout of vertices in a 3D phylogenetic tree. Vertices start with some
initial XYZ layout and then these are adjusted iteratively via FDP algorithm. Note that
Z component is NOT allowed to change so that all displacements are in XY planes. This is
designed to preserve some sense of the depth of vertices in the tree.


Based on pseudocode in  paper by Fruchterman and Reingold 1991.
Uses mySmallTreeLib and data structures from Paloverde for XYZ coords.

PARAMETERS
root		- node for the root of tree
num_iter	- number of iterations moving the vertices
radius 		- radius of the circle in which tips are initially layed out
max_disp 	- maximum allowed displacement of vertex per iteration; decreases each iter
attract_factor	- multiplier of the constant used in the Coulomb replusion equation, to permit
			scaling between repulsion and attraction. Larger numbers = more repulsion.
*/

float FDP(node root,int num_iter,float radius,float max_disp,float attract_factor)

	{
	float Area,D,Fr,Fa,mag_disp,t,dt,F1,F2,max_radius,node_radius,expansion,x,y,z;
				long wA=1, wB=1;
	nodeArray nodes;
	int iter;
	long i,j,n;
	area A,B;


	Area = radius*TWO_PI; 
	gFDP_k = sqrt(Area/root->numdesc);
	gFDP_k_sqr = gFDP_k * gFDP_k;
	gFDP_k1 = attract_factor*gFDP_k;
	t=max_disp;

//	n = numNodes(root);
//	nodes = newNodeArray(root);


	n = numNodesNotSub(root);  // currently only recognizing nodes not descended from compact nodes...
	nodes = newNodeArrayNotSub(root);

	dt=t/num_iter;

	A=(area)((nodes[0])->data); // let's fix the root at (0,0,z) rather than what it is as a result of areaSplit; other nodes will follow suit
	A->x_center=A->y_center=0.0;

	for (iter=1;iter<num_iter;iter++,t-=dt)
		{
		// repulsion:  All pairs of nodes repel each other

		for (i=0;i<n;i++)
			{
			A=(area)((nodes[i])->data);
			A->x_disp=0.0; A->y_disp=0.0;
			for (j=0;j<n;j++)
			    if (i != j)
				{
				B=(area)((nodes[j])->data);
				D=distance(A,B); 		
//				Fr=f_repulsion(D) ;
//				if (nodes[i]->isCompactNode) wA = /*log*/(nodes[i]->numdesc)+1; else wA = 1; // perhaps change to numdescEff later
//				if (nodes[j]->isCompactNode) wB = /*log*/(nodes[j]->numdesc)+1; else wB = 1; // weighting by log size doesn't quite make it

				wA=wB=1; // back to not weighting the clades; the weighting strategy was not as effective as just changing attractfactor

				Fr=f_repulsion(D) * wA * wB;
				F1=Fr/D;
				(A->x_disp)+= (A->x_center-B->x_center)*F1;
				(A->y_disp)+= (A->y_center-B->y_center)*F1;
#if 0
if (nodes[i]->label && nodes[j]->label)
//  if (strcmp(nodes[i]->label,	"Biscutella, etc.")==0  && strcmp (nodes[j]->label,"Austrosteenisia, etc.")==0)	
  if (strcmp(nodes[i]->label,	"g1")==0  && strcmp (nodes[j]->label,"g2")==0)	
	{
	printf("Cherry repulsion\n");
	printf("D=%f F1=%f\n",D, F1);
	}
#endif
				}
			}	
		// attraction: every node is attracted to its ancestor node (i.e., along edges)
		for (i=0;i<n;i++)
			{
			if (!isRoot(nodes[i]))
				{
				A=(area)((nodes[i])->data);
				B=(area)(((nodes[i])->anc)->data);
				D=distance(A,B);
				Fa=f_attraction(D);
				F2=Fa/D;
#if 0
if (nodes[i]->label)
//  if (strcmp(nodes[i]->label,	"Biscutella, etc.")==0  || strcmp (nodes[i]->label,"Austrosteenisia, etc.")==0)	
  if (strcmp(nodes[i]->label,	"g1")==0  || strcmp (nodes[i]->label,"g2")==0)	
	{
	printf("Cherry anc attraction (%s)\n", nodes[i]->label);
	printf("D=%f F1=%f\n",D, F2);
	}
#endif
				(A->x_disp)-= (A->x_center-B->x_center)*F2;
				(A->y_disp)-= (A->y_center-B->y_center)*F2;
				//if (!nodes[i]->marked)
				//	(A->z_disp) -= (A->z-B->z)*F2;

				(B->x_disp)+= (A->x_center-B->x_center)*F2;
				(B->y_disp)+= (A->y_center-B->y_center)*F2;
				//if (!nodes[i]->anc->marked)
				//	(B->z_disp) += (A->z-B->z)*F2;




//printf("Fa=%f D=%f\n",Fa,D);
				}
			}
		// move vertices
            max_radius =  -1000;
		for (i=0;i<n;i++)
		    if (!isRoot(nodes[i])) // the root does not move
				{
				A=(area)((nodes[i])->data);
				mag_disp = sqrt(A->x_disp*A->x_disp + A->y_disp*A->y_disp);
				(A->x_center)+= (A->x_disp/mag_disp)*MIN(mag_disp,t);
				(A->y_center)+= (A->y_disp/mag_disp)*MIN(mag_disp,t);
				//if (!nodes[i]->marked)
				//	(A->z) += (A->z_disp/mag_disp)*MIN(mag_disp,t);



	//printf ("\t%i\t%f\t%f\t%f\t%f\t%f\t%f\n",i,A->x_center,A->y_center,A->z,mag_disp,t,MIN(mag_disp,t));
				if (iter == num_iter-1) // just do this on the last iteration
					{
					node_radius = sqrt(A->x_center*A->x_center+A->y_center*A->y_center);
					if (node_radius>max_radius) max_radius=node_radius;
					}
				}	
		//printf("FDP iteration %i done\n",iter);
		}
	printf ("%i FDP iterations done...\n", iter);
	//printf("Max_radius=%f rescaling...\n",max_radius);
	expansion = max_radius/radius;
	for (i=0;i<n;i++)
			{
			A=(area)((nodes[i])->data);
			(A->x_center) /= expansion;
			(A->y_center) /= expansion;
			//(A->z) /= expansion;

		// bug fix 9/17/2012...I was not updating theta_center after FDP had run! Needed for animateRotation, etc.
			x=A->x_center;
			y=A->y_center;
			z=A->z;
			A->r_center = sqrt (x*x+y*y);
			A->theta_center = atan2(y,x);
			}	
	return (max_radius);
	}

static float distance(area A, area B)
	{
	float xd,yd,zd;
	xd=A->x_center-B->x_center;
	yd=A->y_center-B->y_center;
	zd=A->z-B->z;
	return sqrt(xd*xd+yd*yd+zd*zd);
	//return sqrt(xd*xd+yd*yd);
	}
static float f_attraction(float d)
	{
	return d*d/gFDP_k1;
	}
static float f_repulsion(float d)
	{
	return gFDP_k_sqr/d;
	}

