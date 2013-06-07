#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "my_hash.h"
#include "mySmallTreeLib.h"
#include "tree_openGL.h"
#include "fonts.h"

LabelAttributeObj labelAttributeObjAlloc(void)
	{
    LabelAttributeObj L;
	L = (LabelAttributeObj)malloc(sizeof(struct _LabelAttributeStruct));
	return L;
	}

void labelAttributeObjInit(
		LabelAttributeObj L,
		enum TextStyle	textStyle,
		float			scaleSize,
		short			attenuate,
		float			attenuateFactor,
		float			lod_cutoff,
		enum Alignment	alignment,
		short			suppress,
		GLfloat *		labelColor,
		float			lineWidth,
		enum Orientation orientation,
		short			flipLeftSide,
		float			strokeTerminalOffset,
		void*			font,
		short			diagonalLine
	)
	
	{
		L->textStyle=textStyle;
		L->scaleSize=scaleSize;
		L->attenuate=attenuate;
		L->attenuateFactor=attenuateFactor;
		L->lod_cutoff=lod_cutoff;
		L->alignment=alignment;
		L->labelColor=labelColor;
		L->lineWidth=lineWidth;
		L->orientation=orientation;
		L->flipLeftSide=flipLeftSide;
		L->strokeTerminalOffset=strokeTerminalOffset;
		L->font = font;
		L->diagonalLine = diagonalLine;
		L->suppress = suppress;
	}

LabelAttributeObj labelAttributeObjInitByCopy(
	LabelAttributeObj A
	)
	{
		LabelAttributeObj L;
		L=labelAttributeObjAlloc();
		L->textStyle= A->textStyle;
		L->scaleSize= A->scaleSize;
		L->attenuate= A->attenuate;
		L->attenuateFactor= A->attenuateFactor;
		L->lod_cutoff= A->lod_cutoff;
		L->alignment= A->alignment;
		L->labelColor= A->labelColor;
		L->lineWidth= A->lineWidth;
		L->orientation= A->orientation;
		L->flipLeftSide= A->flipLeftSide;
		L->strokeTerminalOffset= A->strokeTerminalOffset;
		L->font =  A->font;
		L->diagonalLine =  A->diagonalLine;
		L->suppress =  A->suppress;
		return L;
	}


/*
The current schema for drawing labels recognizes two basic types, both implemented with freetype fonts.
1. Horizontal labels that always face viewer. These are displayed attached to a short diagonal line. The size can be controlled
with the l/L key toggle. Although distance labels are smaller, this is modulated by a tweak, LABEL_SIZE_COMPENSATION_FACTOR, which
keeps distant labels larger than they should be. These labels are not attenuated or clipped at a cutoff distance. They may
be displayed at the end of a tube branch, or attached to an image object. 

2. Radial labels that always face viewer (as part of a fan, generally). These are shifted by a distance of 1/2 the vertical
font so that the subtending edge meets the label half way. Size is determined at run time and cannot be changed at the moment. These
labels are attenuated at the same rate as fans are, and are clipped off entirely at a certain LOD distance.

Notes.
	- Labels are truncated to a maximum of TRUNC_LABEL_N characters and if truncated they are padded with "..."

	-WORK TO BE DONE CLEANING THIS UP WRT THE LABEL INITIALIZER CODE, WHICH IS IN A BAD WAY RIGHT NOW. THE ABOVE IS SIMPLER.
	
	-Need to fix the invalid raster issue when horizontal terminal labels go offscreen on left and back on; the scaling factor shifts
	abruptly.
*/

#define LABEL_SIZE_COMPENSATION_FACTOR 100 // modulates decrease in label size at a distance; larger values will keep labels nearer the 

void drawLabelObject(OglTree T, node a, LabelAttributeObj labelAttributes, float xoff, float yoff, float zoff)

// Draw the taxon name, and possibly a translucent box behind the name, and possibly an image above the name
// Current coord is at a's ancestor, pointing with z-axis along branch between a and anc(a). Have to move distance +zr along z axis
// to draw label at node a.

{
	extern float gHalfFontHeight;
	extern float multLabelScale;
	extern GLubyte * gDiagBitmap;
	extern int gImages;
	extern float theta[3];
	float noscale;
	GLboolean valid;
	area A;
	float offset,x,y,z,thetaName,displayNumber, darkness, distance;
	int i,len;
	char trunc[TRUNC_LABEL_N+4]; // remember to terminate this string at last character in case all TRUNC_LABEL_N chars are copied

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

					glRasterPos3f(0,0,0);
					glGetFloatv(GL_CURRENT_RASTER_DISTANCE,params);
					distance=params[0];


					if (T->layout == circle)
						glRotatef(-90,0.0,0.0,1.0);   //context dependent; no rotation when displayed after an image, but yes, after a fan; and mucked up when just a solid tree terminal...
					if (T->layout == solid_cone && !nodeHasImages(a)) // different orientation if it's atop an image
						{
						glRotatef(-theta[2],0.0,0.0,1.0); // ...derotates the tree to face the viewer. Note that the order of this and the next matters; have to reverse the order of the viewing rotation in display()
						glRotatef(-theta[0],1.0,0.0,0.0); //  
						}
					if (T->layout == solid_cone && nodeHasImages(a))
						{
						;
						}
				 
					// Scale the size of the labels, but get fancy and modulate the decline in size in the far distance so labels are still readable
					//float noscale = -labelAttributes->scaleSize*distance/75;
					noscale = 0.005*(1-distance/LABEL_SIZE_COMPENSATION_FACTOR); if (noscale <=0) noscale=0;
					//noscale = labelAttributes->scaleSize;
					noscale *=multLabelScale;
					glScalef(noscale,noscale,noscale); 
		 
					break;
				case face_viewer_radial:
					glRotatef(+toDegs(A->theta_center),0.0,0.0,1.0); // ...rotate names oriented along radii
					// Scale the size of the labels
					noscale = labelAttributes->scaleSize;
					glScalef(noscale,noscale,noscale); // make the labels this size
					break;
				}

			//strncpy(&trunc[0],a->label,TRUNC_LABEL_N);
			//trunc[TRUNC_LABEL_N]='\0';

		truncateLabel(a->label,trunc,TRUNC_LABEL_N);


			labelLen = glutStrokeLength(labelAttributes->font, (unsigned char*)&trunc[0]);
			// make adjustments for flipping labels that are on the left side of circle only
			if (labelAttributes->flipLeftSide)// Do the following corrections for labels on left side of circles only
				{

				thetaName = clamp0_TwoPI(A->theta_center) /*+ theta[2]*/;
				if (inCircularInterval(T->upDirection , clamp0_TwoPI(T->upDirection+PI), thetaName))  // whew, read the function defn.
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
            int ix=0;
			
#if 0
			for (p = a->label; *p; p++)
                {
                if (++ix>TRUNC_LABEL_N) break;
                glutStrokeCharacter(labelAttributes->font, *p);
                }


#else
			switch (labelAttributes->orientation)
				{
				case face_viewer_horiz:
					glColor3f(1,1,1);
	//				glEnable(GL_LINE_STIPPLE);
	//				glLineStipple(1,0xAAAA);
					glBegin(GL_LINES);
						glVertex3f(0,0,0);
						glVertex3f(+gHalfFontHeight,+gHalfFontHeight,0);
					glEnd();
	//				glDisable(GL_LINE_STIPPLE);
					glTranslatef(+gHalfFontHeight,+gHalfFontHeight,0);
					break;
				case face_viewer_radial: 
					glTranslatef(0,-gHalfFontHeight,0);
					break;
					
				}
			render_text(trunc);
#endif
			}
		}
	glDisable(GL_BLEND);

	glPopMatrix();
	return;
}






GLubyte * initBitmapDiagLine(int width) // deprecated
	{

	GLubyte diag[8] = { 0x80,0x40,0x20,0x10,0x8,0x4,0x2,0x1}; // a diagonal in bitspeak
	GLubyte * rasters;
	int i, j,k,offset, dim,idim,ix=0;
	dim = width*width*8;  // size of raster array
	idim= width*8;
	rasters = (GLubyte*)malloc(dim*sizeof(GLubyte)); 
	for (i=1;i<=width;i++)
	   for (k=0;k<8;k++)
		for (j=1;j<=width;j++)
			{
			if (i == j)
				{
				offset = k % 8 ;
				rasters[ix]=diag[offset];
				}
			else
				{
				rasters[ix]=0;
				}

			++ix;
			}
	return rasters;
	}

void drawDiagBitmap(int width, GLubyte * bitmap)
	{
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glPushAttrib(GL_CURRENT_BIT); // save the previous color and raster color, etc.
	glColor4f(0,1,0,1);
	glRasterPos2f(0,0);
	glBitmap(8*width,8*width,0.0,0.0,8*width,8*width,bitmap);
	glPopAttrib();
	}

//make sure to pass in a char array of size maxChars + 1 + numDots

void truncateLabel(char *label, char *truncatedLabel, int maxChars)
	{
	int length = strlen(label);
	if (length <= maxChars)
		strcpy(truncatedLabel,label); // no fuss, just copy
	else
		{
        strncpy(truncatedLabel,label,maxChars);
        truncatedLabel[maxChars]='.';
        truncatedLabel[maxChars+1]='.';
        truncatedLabel[maxChars+2]='.';
        truncatedLabel[maxChars+3]='\0';
		}

	return;
	}



