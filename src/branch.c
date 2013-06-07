#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "my_hash.h"
#include "mySmallTreeLib.h"
//#include "layout.h"
#include "tree_openGL.h"
#include "gle.h"
#include "branch.h"


#define MAX_NURBS_POINTS 1000
GLUnurbsObj *theNurbs;
double gPoint_array[MAX_NURBS_POINTS][3]; // for NURBS stuff REVISIT!
int gNpoints;
GLenum expectedError;




BranchAttributeObj branchAttributeObjAlloc(void)
	{
    BranchAttributeObj B;
	B = (BranchAttributeObj)malloc(sizeof(struct _BranchAttributeStruct));
	return B;
	}

void branchAttributeObjInit(
							BranchAttributeObj		B,
							enum BranchStyle		branchStyle,
							enum BranchCurvy		branchCurvy,
							float					lineWidth,
							GLfloat *				color,
							short					corners,
							short					attenuate,
							float					attenuateFactor,
							float					lod_cutoff,
							float					ctlpointParms[]
							)
	
		{
		int i;
		B->branchStyle=branchStyle;
		B->branchCurvy=branchCurvy;
		B->color=color;
		B->lineWidth=lineWidth;
		B->corners=corners;
		B->attenuate=attenuate;
		B->attenuateFactor=attenuateFactor;
		B->lod_cutoff=lod_cutoff;

		for (i=0;i<NUM_CONTROL_POINT_PARAMS;i++)
			B->ctlpointParms[i]=ctlpointParms[i];

		}


void CALLBACK beginCallback(GLenum whichType)
{
	//glBegin(whichType);
	//printf("\nBegin\n");
	extern int gNpoints;
	gNpoints=0;
}
void CALLBACK endCallback(void)
{
	//glEnd();
	//printf("End\n");
}
void CALLBACK vertexCallback(GLfloat *vertex)
{
	//glVertex3fv(vertex);
	//printf("%f %f %f\n",vertex[0],vertex[1],vertex[2]);
	gPoint_array[gNpoints][0]=vertex[0];
	gPoint_array[gNpoints][1]=vertex[1];
	gPoint_array[gNpoints][2]=vertex[2];
	++gNpoints;
}
void CALLBACK ErrorCallback(GLenum which)  // needed for NURBS stuff?
{
    if (which != expectedError) {
        fprintf(stderr, "Unexpected error occured (%d):\n", which);
        fprintf(stderr, "    %s\n", gluErrorString(which));
    }
}
void nurbsInit(void)
{
    theNurbs = gluNewNurbsRenderer();
    gluNurbsCallback(theNurbs, GLU_ERROR, ErrorCallback);
	gluNurbsProperty(theNurbs,GLU_NURBS_MODE,GLU_NURBS_TESSELLATOR);
	gluNurbsCallback(theNurbs,GLU_NURBS_BEGIN,beginCallback);
	gluNurbsCallback(theNurbs,GLU_NURBS_END,endCallback);
	gluNurbsCallback(theNurbs,GLU_NURBS_VERTEX,vertexCallback);
    gluNurbsProperty(theNurbs, GLU_SAMPLING_TOLERANCE, 15.0);
    gluNurbsProperty(theNurbs, GLU_DISPLAY_MODE, GLU_OUTLINE_PATCH);
    expectedError = GLU_INVALID_ENUM;
    gluNurbsProperty(theNurbs, ~0, 15.0);
    expectedError = GLU_NURBS_ERROR13;
    gluEndSurface(theNurbs);
    expectedError = 0;

}
// ***********************************
//

void drawBranchObject(node a, BranchAttributeObj branchAttributes, float radius1,float radius2)
// Draw a cylinder subtending a node, using precomputed values from trig routine
// This is currently the only place in the code I need GL_LIGHTING on

// This is drawn in the original object coordinate system in which the tree is centered on 0,0

// TO DO enable attenuate on/off flag

// Not doing blending or lod_cutoff for tubes yet
// Currently doing the lod_cutoff for branchStyle==line only ; makes a big difference


	{
	extern GLUquadricObj *qobj;
	GLfloat params[4];
	GLboolean valid;
	float darkness;
	float xdist,ydist,zdist;
	float sknots[8] = { 0,0,0,0,1,1,1,1};
	GLfloat ctlpoints[4][4]; // four control points = 2 end points + two "control" points
	area A,Aanc;
	double dx,dy,dz,zr,xcross,ycross,theta, color, color_anc;
	A=(area)(a->data);
	float r1,r2,z1,z2;

	if (branchAttributes->branchCurvy == nurbs) // set up control points
		{
		// TO DO. CURRENTLY THESE CONTROL POINTS ARE ALL IN A VERTICAL PLANE. MAKE THE TREE CURVIER YET BY MOVING THEM OFF THIS PLANE.

		// Setting up the two endpoints of the edge; cubic spline will interpolate these two points
		// NB. These are bogus for circle layout, because there is no variation in the z-axis there,


				ctlpoints[0][0]=A->x_anc; ctlpoints[0][1]=A->y_anc; ctlpoints[0][2]=A->z_anc; ctlpoints[0][3]=1.0; 
				ctlpoints[3][0]=A->x_center; ctlpoints[3][1]=A->y_center; ctlpoints[3][2]=A->z; ctlpoints[3][3]=1.0; 


		// Setting up the two control points. These are each tweaked with a z parameter and an r parameter in the vertical plane
		// defined by the two endpoints of the edge, such that both when (r,z) is (0,0) at anc node and (1,1) at desc node.
		// So we need an (r,z) for each control point: r1,z1,r2,z2.

				xdist = A->x_center - A->x_anc;
				ydist = A->y_center - A->y_anc;
				zdist = A->z - A->z_anc;
extern float gr1,gr2,gz1,gz2;
				r1 = gr1; //branchAttributes->ctlpointParms[0];
				z1 = gz1; //branchAttributes->ctlpointParms[1];
				r2 = gr2; //branchAttributes->ctlpointParms[2];
				z2 = gz2; //branchAttributes->ctlpointParms[3];

				ctlpoints[1][0]=A->x_anc + xdist*r1; 
				ctlpoints[1][1]=A->y_anc + ydist*r1; 
				ctlpoints[1][2]=A->z_anc + zdist*z1;
				ctlpoints[1][3]=1.0; 
				
				ctlpoints[2][0]=A->x_anc + xdist*r2; 
				ctlpoints[2][1]=A->y_anc + ydist*r2; 
				ctlpoints[2][2]=A->z_anc + zdist*z2;
				ctlpoints[2][3]=1.0; 
		
		}

	switch (branchAttributes->branchStyle)
		{
		case tube:
			{

			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);


			switch (branchAttributes->branchCurvy)
				{
				case nurbs:
					{
						

						if (a->marked)
							{
							darkness=1;
							glEnable(GL_BLEND); 
							glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
							}
						else
							darkness = setColorWithAttenuationLineSegment(A->x_center,A->y_center,A->z, A->x_anc,A->y_anc,A->z_anc, &(branchAttributes->color[0]), branchAttributes->attenuateFactor);


			// Turn off actual rendering here...no need to draw the line, just get callbacks...use GLU_tesselator
						gluNurbsProperty(theNurbs,GLU_NURBS_MODE,GLU_NURBS_TESSELLATOR);
						gluBeginCurve(theNurbs);  
						gluNurbsCurve(theNurbs, 8, sknots, 4, &ctlpoints[0][0], 4, GL_MAP1_VERTEX_4);
						gluEndCurve(theNurbs);
						gluNurbsProperty(theNurbs,GLU_NURBS_MODE,GLU_NURBS_RENDERER);

						int i;
						float gleColor[500][4];
						double gleRadius[500];
						for (i=0;i<gNpoints;i++) // set the vectors of colors and radii
							{
							if (gNpoints > 500) exit(1);

							gleColor[i][0]= branchAttributes->color[0];
							gleColor[i][1]= branchAttributes->color[1];
							gleColor[i][2]= branchAttributes->color[2];
							gleColor[i][3]= darkness;  // don't seem to need this shit; taken care of by glMaterial
							if (gNpoints > 1)
								gleRadius[i] = radius1 + (radius2-radius1)*i/(gNpoints-1); // interpolate to vary radius between nodes of branch
							else
								gleRadius[i] = radius1;

							}
					params[0]=branchAttributes->color[0]; 
					params[1]=branchAttributes->color[1]; 
					params[2]=branchAttributes->color[2]; 
					params[3]=darkness;

					params[3]=1.0; //override for now; I don't like how the blending looks


							glEnable(GL_BLEND); 
							glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
						//glEnable(GL_LINE_SMOOTH);
						//glHint(GL_LINE_SMOOTH_HINT,GL_NICEST); // Antialiasing not working in this gle environment
						//glEnable(GL_POLYGON_SMOOTH);
						//glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
				glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, params);
						gleSetJoinStyle(TUBE_JN_ROUND) ;
						//glePolyCylinder(gNpoints,gPoint_array,gleColor,radius2);

						//glePolyCone_c4f(gNpoints,gPoint_array,gleColor,gleRadius);
						// Note if we pass gleColor, that overrides the material color and its alpha blending; 
						// so to enable blending pass NULL in place of gleColor, and set glMaterial above
						glePolyCone_c4f(gNpoints,gPoint_array,NULL,gleRadius);

					//drawCylinder(gPoint_array[0][0],gPoint_array[0][1],gPoint_array[0][2], gPoint_array[1][0],gPoint_array[1][1],gPoint_array[1][2],radius1, radius1); // add gumby cyls at ends of gle cylinder
					//drawCylinder(gPoint_array[gNpoints-2][0],gPoint_array[gNpoints-2][1],gPoint_array[gNpoints-2][2], gPoint_array[gNpoints-1][0],gPoint_array[gNpoints-1][1],gPoint_array[gNpoints-1][2],radius2, radius2);
		
					// Because gle does not extrude at the two endpoints, I draw cylinders at each endpoint. To avoid gappiness at borders, I overlap the cylinder with the extruded tube by one segment (one point on the polyline), thus I run the cylinder from, e.g., point 0 to point 2, where the extrusion starts at 1.
					drawCylinder(gPoint_array[0][0],gPoint_array[0][1],gPoint_array[0][2], gPoint_array[2][0],gPoint_array[2][1],gPoint_array[2][2],gleRadius[0], gleRadius[2]); // add gumby cyls at ends of gle cylinder
					drawCylinder(gPoint_array[gNpoints-3][0],gPoint_array[gNpoints-3][1],gPoint_array[gNpoints-3][2], gPoint_array[gNpoints-1][0],gPoint_array[gNpoints-1][1],gPoint_array[gNpoints-1][2],gleRadius[gNpoints-3],gleRadius[gNpoints-1]);


					//glDisable(GL_LIGHTING);
					break;
					}
				case straight:
					{
					params[0]=branchAttributes->color[0]; 
					params[1]=branchAttributes->color[1]; 
					params[2]=branchAttributes->color[2]; 
					params[3]=darkness;
					glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, params);
					drawCylinder(A->x_anc,A->y_anc,A->z_anc, A->x_center,A->y_center,A->z, radius1, radius2);
					break;
					}
				}
			glDisable(GL_LIGHTING);
			break;
			}

	

		case line:
			{
			if (!isRoot(a))
				{
						glLineWidth(branchAttributes->lineWidth);
			
						if (a->marked)
							{
							darkness=1;
							glEnable(GL_BLEND); 
							glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
							glColor4f(branchAttributes->color[0],branchAttributes->color[1],branchAttributes->color[2],darkness);
							}
						else
							darkness = setColorWithAttenuationLineSegment(A->x_center,A->y_center,A->z, A->x_anc,A->y_anc,A->z_anc, &(branchAttributes->color[0]), branchAttributes->attenuateFactor);

				// there is similar code in function setColorWithAttenuation() in tree_openGL, but here we have something more elaborate...


				switch (branchAttributes->branchCurvy)
					{
					case nurbs:
						{


						if (branchAttributes->lod_cutoff < darkness)
							{
							gluNurbsProperty(theNurbs,GLU_NURBS_MODE,GLU_NURBS_RENDERER);
							gluBeginCurve(theNurbs);
							gluNurbsCurve(theNurbs, 8, sknots, 4, &ctlpoints[0][0], 4, GL_MAP1_VERTEX_4);
							gluEndCurve(theNurbs);
							}
							break;
						}
					
					case straight:
						{

						if (branchAttributes->lod_cutoff < darkness)
							{
							glBegin(GL_LINES);
							glVertex3d(A->x_anc,A->y_anc,A->z_anc);
							glVertex3d(A->x_center,A->y_center,A->z);
							glEnd();
							}
						}
				}
			break;
			}
		}
	}
	}
	
/*
Attenuates the brightness of a line segment between two points. If one point is off screen, attenuate correctly.  Return darkness value for later use.
*/
float setColorWithAttenuationLineSegment(float x1, float y1, float z1, float x2, float y2, float z2, GLfloat color[], float attenuateFactor)
	{
	GLfloat params[4];
	GLboolean valid;
	float darkness;


	glRasterPos3f(x1,y1,z1); // unlike other places in code, here we have to set the raster
		// to the real coordinates in the universal coord frame, rather than e.g., 0,0 in the local frame.
		// Later, it might be nice to take the midpoint of the branch!
	glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,&valid);
	if (valid) // sometimes one end of the branch is off screen, causing eye distance to be undefined...
		{
		glGetFloatv(GL_CURRENT_RASTER_DISTANCE,params);
		darkness = 1 + attenuateFactor*params[0]/1000;
		}
	else
		{					
		glRasterPos3f(x2,y2,z2);
		glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,&valid);
		if (valid)
			{
			glGetFloatv(GL_CURRENT_RASTER_DISTANCE,params);
			darkness = 1 + attenuateFactor*params[0]/1000; // use the other endpoint if it is onscreen
			}
		else
			darkness = 0.0;  // case where both ends are offscreen; OGL should handle this anyway.
		}
		
	if (darkness > 1) darkness = 1;
	if (darkness < 0) darkness = 0;

	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(color[0],color[1],color[2],darkness);

	return darkness;
	}
	
	
// ***********************************

void drawCornersArcBranch(node a, OglTree T, float radius)
	{
	float ntheta, minTheta=1000, maxTheta=-1000, rR, zR, zL;
	node n;
	area nA,A;
	if (!isTip(a) && !isRoot(a) && (!a->isCompactNode)) // draw the arcs...
		{
		A=(area)(a->data);
		n=a->firstdesc;
		SIBLOOP(n)
			{
			nA=(area)(n->data);
			ntheta=nA->theta_center;
			if (ntheta>maxTheta) maxTheta=ntheta;
			if (ntheta<minTheta) minTheta=ntheta;	
			}
		rR = A -> r1; // ??
		zR = zL = A -> z; // try this for goblet
		drawArcBranch(minTheta, rR, zR, maxTheta, rR, zL, radius,T->branchAttributes); // fix last arg !!!!!
		}

	
	}

void drawArcBranch(double theta1, double r1, double z1, double theta2, double r2, double z2,  float radius, BranchAttributeObj branchAttributes)

// theta1,2 should be in radians!

	{
#define MAXARCSEG 100
	int numCyl,numPts,ix=0,i; // determine adaptively!
	float darkness;
	double v[MAXARCSEG][3],dth,dr,dz,theta,r,z,x,y;
	numCyl = fabs(100*(theta2-theta1)/TWO_PI); // poor adaptive calc for number of segments (cylinders) in arc
	if (numCyl==0) numCyl=1;
	numPts=numCyl+1;
	if (numPts >= MAXARCSEG)
		fatal("Array bounds were exceeded in drawArcBranch\n");
	dth = (theta2-theta1)/numCyl;	
	dr = (r2-r1)/numCyl;	
	dz = (z2-z1)/numCyl;	
	for (i=0;i<numPts;i++)
		{
		theta = theta1 + dth*i;
		r = r1 + dr*i;
		z = z1 + dz*i;
		x = r * cos (theta);
		y = r * sin (theta);
		v[i][0]=x;
		v[i][1]=y;
		v[i][2]=z;
		}
	// attenuate using the two endpoints of the entire arc as a "line segment"
	darkness = setColorWithAttenuationLineSegment(v[0][0],v[0][1],v[0][2],v[numPts-1][0],v[numPts-1][1],v[numPts-1][2], &(branchAttributes->color[0]), branchAttributes->attenuateFactor);

	if (branchAttributes->lod_cutoff < darkness)
		for (i=1;i<numPts;i++)
				{
				if (branchAttributes->branchStyle == tube)
					drawCylinder(v[i-1][0],v[i-1][1],v[i-1][2],v[i][0],v[i][1],v[i][2],radius,radius);
				if (branchAttributes->branchStyle == line)
					{
					glLineWidth(branchAttributes->lineWidth);
					glBegin(GL_LINES);
						glVertex3d(v[i-1][0],v[i-1][1],v[i-1][2]);
						glVertex3d(v[i][0],v[i][1],v[i][2]);
					glEnd();
					}
				}
	}
//
//


	
