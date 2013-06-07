#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "my_hash.h"
#include "mySmallTreeLib.h"
#include "tree_openGL.h"


DataObj dataObjAlloc(void)
	{
    DataObj L;
	L = (DataObj)malloc(sizeof(struct _dataObjStruct));
	return L;
	}

void dataObjInit(
	DataObj			L,
	enum ImageType	dataType,
	float			theta[3],
	float			boxWidth,
	short			attenuate,
	float			attenuateFactor,
	float			lod_cutoff,
	enum FaceType	faces;
	)
	
	{
		L->dataType=dataType;
		L->theta[0]=theta[0];
		L->theta[1]=theta[1];
		L->theta[2]=theta[2];
		L->boxWidth=scaleSize;
		L->attenuate=attenuate;
		L->attenuateFactor=attenuateFactor;
		L->lod_cutoff=lod_cutoff;
		L->faces=faces;
	}

DataObj dataObjInitByCopy(
	DataObj A
	)
	{
		DataObj L;
		L=dataObjAlloc();
		L->boxWidth= A->boxWidth;
		L->attenuate= A->attenuate;
		L->attenuateFactor= A->attenuateFactor;
		L->lod_cutoff= A->lod_cutoff;
		L->faces= A->faces;
		return L;
	}



void drawLabelObject(DataObj L, float x, float y, float z)


{
extern GLubyte * gDiagBitmap;
extern float theta[3];
GLboolean valid;
area A;
float offset,x,y,z,thetaName,displayNumber, darkness, distance;
int i,len;

if (!a->label) return;
A=(area)(a->data);
GLfloat params[4];
int labelLen;
char *p;

if (labelAttributes->suppress)
	return;

glPushMatrix();
glTranslatef(xoff,yoff,zoff);
glLineWidth(labelAttributes->lineWidth);
switch (labelAttributes->textStyle)  
	{
	case stroke: 
		glTranslatef(0.0,0.0,+labelAttributes->strokeTerminalOffset);
		//glTranslatef(0.0,0.0,(A->zr)+labelAttributes->strokeTerminalOffset);
		switch (labelAttributes->orientation)
			{
			case face_viewer_horiz:
				glRotatef(-A->theta,A->cross_prodx,A->cross_prody,0.0); // puts the labels in one plane and orientation
				glRotatef(-theta[2]-gP->Heading,0.0,0.0,1.0); // ...and then moves them to face the viewer. Note that the order 
					  // of this and the next call is important! (for some reason) 
				glRotatef(-theta[0]+gP->Pitch,1.0,0.0,0.0); // to ensure that the labels keep the same orientation toward the viewer
				break;
			case face_viewer_radial:
				//glRotatef(-A->theta,A->cross_prodx,A->cross_prody,0.0); // puts the labels in one plane and orientation
				glRotatef(+toDegs(A->theta_center),0.0,0.0,1.0); // ...rotate names oriented along radii
				break;
			}

		// Scale the size of the labels
		glScalef(labelAttributes->scaleSize,labelAttributes->scaleSize,labelAttributes->scaleSize); // make the labels this size
		labelLen = glutStrokeLength(labelAttributes->font, (unsigned char*)(a->label));
		// make adjustments for flipping labels that are on the left side of circle only
		if (labelAttributes->flipLeftSide)
			{
			thetaName = toDegs(A->theta_center) /*+ theta[2]*/;
			if (thetaName > 360.0) thetaName -=360.0;
			if (thetaName < 0.0) thetaName +=360.0;
			if (thetaName > 90 && thetaName < 270)  
				// Do the following corrections for labels on left side of screen only
				  {
					glTranslatef(+labelLen,0.0,0.0);
					glRotatef(180,0.0,0.0,1.0);
				  }
			}
		break;
	case bitmap:  
		//glTranslatef(0.0,0.0,(A->zr));
		len = (int) strlen(a->label);
		break;
	}




	glRasterPos3f(0,0,0);
	glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,&valid);
	if (!valid) // sometimes one end of label is off screen, causing eye distance to be undefined...
		{
		glRasterPos3f(labelLen,0,0); // Move back to the node and use its raster position instead;
		glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,&valid);
		if (valid)
			{
			if (labelAttributes->attenuate)
				{
				glGetFloatv(GL_CURRENT_RASTER_DISTANCE,params);
				distance=params[0];
				darkness = 1 + labelAttributes->attenuateFactor*distance/1000;
				}
			else
				darkness = 1.0;
			}
		else
			darkness = 0.0;
		}
	else
		{					
		if (labelAttributes->attenuate)
			{
			glGetFloatv(GL_CURRENT_RASTER_DISTANCE,params);
			distance=params[0];
			darkness = 1 + labelAttributes->attenuateFactor*distance/1000;
			}
		else
			darkness = 1.0;
		}
		

	if (darkness > 1) darkness = 1;
	if (darkness < 0) darkness = 0;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(labelAttributes->labelColor[0],labelAttributes->labelColor[1],labelAttributes->labelColor[2],darkness);
	glRasterPos3f(0,0,0);		// Do this again to update color stuff! command resets colors! (see Redbook:308); so 	
	if (darkness > labelAttributes->lod_cutoff)	 // eliminates occlusion of visible labels by some nearly invisible labels anyway
		{
		if (labelAttributes->textStyle == bitmap)
			{
			
			if (labelAttributes->diagonalLine)
				{
				drawDiagBitmap(1,gDiagBitmap);
				glBitmap(0,0,0,0,8+2,8+2,NULL); // have to actually move raster to get ready to display label
				}
			if (labelAttributes->alignment == center)
				glBitmap(0,0,0,0,-glutBitmapLength(labelAttributes->font,a->label)/2.,0,NULL);

			for (i = 0; i < len; i++)
				glutBitmapCharacter(labelAttributes->font, (a->label)[i]); // keep this font size
			}
		if (labelAttributes->textStyle == stroke)
			{
			for (p = a->label; *p; p++)
				glutStrokeCharacter(labelAttributes->font, *p);
			}
		}
	glDisable(GL_BLEND);

glPopMatrix();
return;
}



