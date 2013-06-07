/* 



Use the function test nodeHasImages(node a) to see if a node has ANY images.

---MODIFIED by MJS to read a PPM file but order the rows from bottom
to top, as needed by OpenGL bitmap operations. PPM files are ordered
from top to bottom.

 * glmReadPPM: read a PPM raw (type P6) file.  The PPM file has a header
 * that should look something like:
 *
 *    P6
 *    # comment
 *    width height max_value
 *    rgbrgbrgb...
 *
 * where "P6" is the magic cookie which identifies the file type and
 * should be the only characters on the first line followed by a
 * carriage return.  Any line starting with a # mark will be treated
 * as a comment and discarded.   After the magic cookie, three integer
 * values are expected: width, height of the image and the maximum
 * value for a pixel (max_value must be < 256 for PPM raw files).  The
 * data section consists of width*height rgb triplets (one byte each)
 * in binary format (i.e., such as that written with fwrite() or
 * equivalent).
 *
 * The rgb data is returned as an array of unsigned chars (packed
 * rgb).  The malloc()'d memory should be free()'d by the caller.  If
 * an error occurs, an error message is sent to stderr and NULL is
 * returned.
 *
 * filename   - name of the .ppm file.
 * width      - will contain the width of the image on return.
 * height     - will contain the height of the image on return.
 *
 */
#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tree_openGL.h"
#include "getPPM.h"



extern int gRenderBin;
node gPPM_Test_Node; // for testing purposes just display this leaf node's image on all leaves;

int nodeHasImages(node a)
	{
	if (a->data2)
		return 1;
	else
		return 0;
	}

ppm_image getPPM(char* filename)
{
    FILE* fp;
    ppm_image ppm;
    int i, w, h, d, row;
    unsigned long length;
    unsigned char* image;
    char head[70];          /* max line <= 70 in PPM (per spec). */
    
    fp = fopen(filename, "rb");
    if (!fp) {
        perror(filename);
        return NULL;
    }
    
    /* grab first two chars of the file and make sure that it has the
       correct magic cookie for a raw PPM file. */
    fgets(head, 70, fp);
    if (strncmp(head, "P6", 2)) {
        fprintf(stderr, "%s: Not a raw PPM file\n", filename);
        return NULL;
    }
    /* grab the three elements in the header (width, height, maxval). */
    i = 0;
    while(i < 3) {
        fgets(head, 70, fp);
	//printf("%s:%s\n",filename,head);
        if (head[0] == '#')     /* skip comments. */
            continue;
        if (i == 0)
            i += sscanf(head, "%d %d %d", &w, &h, &d);
        else if (i == 1)
            i += sscanf(head, "%d %d", &h, &d);
        else if (i == 2)
            i += sscanf(head, "%d", &d);
    }
 
    length=w*h*3;
    image = (unsigned char*)malloc(sizeof(unsigned char)*w*h*3);
    //fread(image, sizeof(unsigned char), w*h*3, fp); Nate Robbins version...

    for (row=1;row<=h;row++) // ...mine
	{
	fread(image+length-row*w*3,sizeof(unsigned char),w*3,fp);
	}

    fclose(fp);

    ppm=(ppm_image)malloc(sizeof(struct ppmstruct));
    if (ppm)
	{
	    ppm->image=image;
	    ppm->width=w;
	    ppm->height=h;    
	    return ppm;
	}
    else
	return NULL;
}



void renderPPM(ppm_image ppm, int face, float boxwidth)
	{
	float x,y,z;
	z=y=x=boxwidth/2.0;
extern int gMaxImagesPerNode;

if (gMaxImagesPerNode==1) z=0; // hack to blow off boxes for single images...
	
			if (ppm)
				{
				glDisable(GL_BLEND);
				glEnable(GL_TEXTURE_2D);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				glBindTexture(GL_TEXTURE_2D,ppm->texName);


//glEnable(GL_CULL_FACE); // culls the back face but then its invisible -- I'd need to put a shaded square or the like if I want this...
//glCullFace(GL_BACK);
GLuint params[1];
glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_RESIDENT,params);
if (params[0] != GL_TRUE) printf("Texture is not resident\n");
				
				glBegin(GL_QUADS);
				switch (face)
					{
					case 0: //front face 
						glTexCoord2f(0,0);  glVertex3f(-x,-y,+z); 
						glTexCoord2f(1,0);  glVertex3f(+x,-y,+z);
						glTexCoord2f(1,1);  glVertex3f(+x,+y,+z);
						glTexCoord2f(0,1);  glVertex3f(-x,+y,+z);
						break;
					
					case 1: //right face
						glTexCoord2f(0,0);  glVertex3f(+x,-y,+z); 
						glTexCoord2f(1,0);  glVertex3f(+x,-y,-z);
						glTexCoord2f(1,1);  glVertex3f(+x,+y,-z);
						glTexCoord2f(0,1);  glVertex3f(+x,+y,+z);
						break;
					
					case 2: // back face
						glTexCoord2f(0,0);  glVertex3f(+x,-y,-z); 
						glTexCoord2f(1,0);  glVertex3f(-x,-y,-z);
						glTexCoord2f(1,1);  glVertex3f(-x,+y,-z);
						glTexCoord2f(0,1);  glVertex3f(+x,+y,-z);
						break;
					
					case 3: // left face
						glTexCoord2f(0,0);  glVertex3f(-x,-y,-z); 
						glTexCoord2f(1,0);  glVertex3f(-x,-y,+z);
						glTexCoord2f(1,1);  glVertex3f(-x,+y,+z);
						glTexCoord2f(0,1);  glVertex3f(-x,+y,-z);
						break;
					
					case 4:  // top face
						glTexCoord2f(0,0);  glVertex3f(-x,+y,+z); 
						glTexCoord2f(1,0);  glVertex3f(+x,+y,+z);
						glTexCoord2f(1,1);  glVertex3f(+x,+y,-z);
						glTexCoord2f(0,1);  glVertex3f(-x,+y,-z);
						break;
					
					case 5: // bottom face
						glTexCoord2f(0,0);  glVertex3f(-x,-y,-z); 
						glTexCoord2f(1,0);  glVertex3f(+x,-y,-z);
						glTexCoord2f(1,1);  glVertex3f(+x,-y,+z);
						glTexCoord2f(0,1);  glVertex3f(-x,-y,+z);
						break;
					
					}
				glEnd();

				
				glDisable(GL_TEXTURE_2D);	
				glEnable(GL_BLEND);

//glDisable(GL_CULL_FACE);



				}
	}

void initTexByNode(node n) // initialize a texture object at a node iff that node has a label (and hence an image)
	{
	extern int gCompressImages, gMaxImagesPerNode;
	int fix;
	GLuint texName;
	GLenum error;
	ppm_image ppm;
	ppm_image *image_array;
	if (nodeHasImages(n)) 
		{
#if PPM_TESTING
		image_array=(ppm_image*)(gPPM_Test_Node->data2);
#else
		image_array=(ppm_image*)(n->data2);
#endif


		if (image_array) // superfluous
			{
			for (fix = 0; fix<gMaxImagesPerNode; fix++)
//			for (fix = 0; fix<1; fix++)
				{
				ppm=image_array[fix];
				if (ppm)
					{
					glGenTextures(1,&texName);
					ppm->texName=texName;
					glBindTexture(GL_TEXTURE_2D, texName);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // !OUTSTANDING; this option finally works...
					if (gCompressImages)
						error = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_COMPRESSED_RGB, ppm->width, ppm->height, GL_RGB, GL_UNSIGNED_BYTE, ppm->image);
					else
						error = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, ppm->width, ppm->height, GL_RGB, GL_UNSIGNED_BYTE, ppm->image);
					// compressed rgb really makes a difference in memory management for residence issues
					if (error) printf ("Mipmaps did not build in ppm: error = %s\n",gluErrorString(error));

					// Notes on compression. I checked the actual internal format stored, and in OSX 10.7 it is using an openGL extension 
					// format documented in glext.h rather than gl.h. Hard to track down.

					}
				}
			}
		}
	}

void drawPPM(DrawEnum renderFlag, OglTree T, node a, float boxW)

// Draw the taxon name, and possibly a translucent box behind the name, and possibly an image above the name
// Current coord is at a's ancestor, pointing with z-axis along branch between a and anc(a). Have to move distance +zr along z axis
// to draw label at node a.

			{
			int style = 0; // 0 = 3d and font; 1 = bitmap, rasterized
			extern GLfloat lt_yellow[],green[],earth_blue[],blue[],black[],red[],white[],brown[],lt_green[];
			extern float objTheta[3];
			area A;
			float offset,x,y,z,thetaName,displayNumber, darkness, distance;
			extern float gTermOffset;
			extern double gRadiusOfOriginalCircle;
			extern enum LayoutChoice gLayoutChoice;
			extern int gLabelFace, gMaxImagesPerNode;
			//extern plane gP;
			extern float theta[3];
			int i,len,fix;
			if (!a->label) return;
			enum LayoutChoice layout = T->layout;
			
#if PPM_TESTING
			if (!gPPM_Test_Node->data2) return;
			A=(area)(a->data);
			ppm_image ppm;
			ppm_image * image_array = gPPM_Test_Node->data2;
#else
			if (!nodeHasImages(a)) return; //no images at this node
			A=(area)(a->data);
			ppm_image ppm;
			ppm_image * image_array = a->data2;
#endif
			GLfloat params[4];
			int labelLen;
			char *p, labelBuf[128];

			glPushMatrix();  

			// change to local coords centered at the node and in right orientation
			//glTranslatef(0.0,0.0,(A->zr)+gTermOffset);
			switch (layout)
				{
				case cone:
				case hemisphere:
				case solid_cone:
#if 0
					if (IMAGES_PER_LEAF>1)
							{
							/* Don't billboard for the box view
							Rotate the coord system so that +z for the box is coming out of the screen, +y is up and +x is to the right.
							Then we will put the 0 face closest to the user, and 1 to its immediate right, etc.*/
							glRotatef(+90,1.0,0.0,0.0);  
							glTranslatef(0,+boxW/2,0);	// but elevate it above the node just enough 
							}
					else
#endif
						{//billboard when single image
							glRotatef(-theta[2]-gP->Heading,0.0,0.0,1.0); // ...and then moves them to face the viewer. Note that the order 
								  // of this and the next call is important! (for some reason) 
							glRotatef(-theta[0]+gP->Pitch,1.0,0.0,0.0); // to ensure that the labels keep the same orientation toward the viewer
							glTranslatef(0,+boxW/2,0);	// and elevate it above the node just enough (notice now along y axis)
						}
					break;
				case circle:
					glRotatef(-A->theta,A->cross_prodx,A->cross_prody,0.0); // puts the labels in one plane and orientation
					glRotatef(+toDegs(A->theta_center),0.0,0.0,1.0); // ...rotate names oriented along radii
					break;
	}


// make adjustments for flipping labels that are on the left side of circle only
			if (layout == circle)
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

			if (gRenderBin==3)
				if (renderFlag == RENDERING)
					if (gMaxImagesPerNode == 1)
						drawLabelObject(T, a, T->terminalLabelAttributes,+boxW/2,+boxW/2,0); 
					else
						drawLabelObject(T, a, T->terminalLabelAttributes,+boxW/2,+boxW/2,+boxW/2);


			// these are rotations of the images themselves in response to alt-arrow keys
			glRotatef(objTheta[0],1,0,0);
			glRotatef(objTheta[1],0,1,0);


			if (gRenderBin==2)
				{
				if (gMaxImagesPerNode == 1)
					{
					if (renderFlag==RENDERING)
						{
								ppm=image_array[0];
								if (ppm)
									{
									renderPPM(ppm,0,boxW);
									}
						}
					if (renderFlag==PICKING)
						{
						drawSquare(boxW);
						}
					}
				else
					{
					if (renderFlag==RENDERING)
						for (fix = 0; fix<gMaxImagesPerNode; fix++)
								{
								ppm=image_array[fix];
								if (ppm)
									{
									renderPPM(ppm,fix,boxW);
									}
								}
					drawTranslucentBox3d(renderFlag,boxW,boxW, boxW, 0.2);

					}
							
				}
				glPopMatrix();
				return;
}


