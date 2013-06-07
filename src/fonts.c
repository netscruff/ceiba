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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tree_openGL.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "fonts.h"

/*
Now using freetype fonts as texture maps. Lots of pros and cons.
-Using alpha blending for transparency but lingering issues with sorting depth and blending (usual story)
-Using multisampling for antialiasing. Otherwise rotations make the text look poor. Also, poor alignment of texture
	map with respect to pixel boundaries is making edge of glyphs bad all over the place.
-Don't have much control over multisampling with GLUT, need GL 3 or 4 probably
-Currently hardwired font directory at /Library/Fonts/
-Using some primitive kerning to correct for overlarge space and underscore characters. I cut these in half.
-I DON'T THINK MIPMAPPING WORKING CORRECTLY. STILL SEE SHIMMERING ARTIFACTS ON DISTANT LABELS
-


*/



// I DON'T THINK MIPMAPPING IS WORKING YET...BUT MULTISAMPLING FOR ANTIALIASING IS.

// Note special handling of space characters
#define MIN_ASCII 32
#define MAX_ASCII 126  // Range of ascii values we care about, including space=32

#define SPACE_KERNING 2.0 // the space char by default is too wide, so implement simple kerning by cutting it in half

FT_Library ft;
FT_Face face;
int fontWidthArray[MAX_ASCII+1];
FT_BitmapGlyph glyf_bitmap[MAX_ASCII+1];
GLuint fontTex[MAX_ASCII+1];
float gHalfFontHeight; // used to correct position of font

// gcc test.c -I /usr/local/include/freetype2/ -lfreetype


// Code adapted from the text01 demo on the modern OpenGL wiki book
// but I am remaining with the fixed function pipeline and blowing off their OGL 2+ stuff,
// and not much remains of their code.


int font_init() // initializes freetype and also sets up my half-baked scheme to cache all needed bitmaps
				// rather than using their API...caveat emptor. This does not yet do any openGL stuff because
				// I want to use this BEFORE I initialized OGL, in layout module
				// Non OpenGL stuff
	{
	extern char gFontLocation[256];

	FT_Glyph aglyf;
	int c;
	 
	if(FT_Init_FreeType(&ft)) {
	  fprintf(stderr, "Could not init freetype library\n");
	  exit(1);
		}

	//if(FT_New_Face(ft, "/Library/Fonts/Arial.ttf", 0, &face)) {
	if(FT_New_Face(ft, gFontLocation, 0, &face)) {
	  fprintf(stderr, "Could not open font\n");
	  exit(1);
		}

	FT_GlyphSlot g = face->glyph;
	FT_Set_Pixel_Sizes(face, 0, PIXEL_SIZE);
	FT_BitmapGlyph bmg;

	for (c=MIN_ASCII;c<=MAX_ASCII;c++) {
		/* Try to load and render the character */
		int error = (FT_Load_Char(face, (char)c, FT_LOAD_RENDER));
		if (error)
			printf ("Error loading character in font init routine\n");


		fontWidthArray[c] = g->advance.x >> 6;
		error = FT_Get_Glyph( face->glyph, &aglyf );
		error = FT_Glyph_To_Bitmap( &aglyf,  FT_RENDER_MODE_NORMAL, NULL, 1 );
		bmg = glyf_bitmap[c] = (FT_BitmapGlyph)aglyf;
		}
	bmg = glyf_bitmap['M'];
	gHalfFontHeight = bmg->bitmap.rows/2.0; // base it on the standard M character used for font faces
	}

int font_init_tex() // openGL stuff
	{
	FT_BitmapGlyph bmg;
	int c;
	glGenTextures(MAX_ASCII+1, fontTex);
	for (c=MIN_ASCII;c<=MAX_ASCII;c++) {
	/* Try to load and render the character */
	bmg = glyf_bitmap[c];

	glBindTexture(GL_TEXTURE_2D, fontTex[c]);

	/* We require 1 byte alignment when uploading texture data */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* Clamping to edges is important to prevent artifacts when scaling */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // IMPORTANT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Linear filtering usually looks best for text */
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	//glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, bmg->bitmap.width, bmg->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bmg->bitmap.buffer);

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR ); // worked for PPM shimmering artifacts but not here


	GLenum error=gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA, bmg->bitmap.width, bmg->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, bmg->bitmap.buffer);
	//if (error) printf ("Mipmaps did not build in font init tex for char (int)=%i: error = %s (%i %i)\n",c,gluErrorString(error), bmg->bitmap.width, bmg->bitmap.rows);

		}
	
	}



// Return length of string in pixels (Freetype)

float myFTStringLength(const char *text)
	{
	float x=0.0;
	const char *p;
	for (p = text; *p; p++)
		if (*p=='_' || *p==' ')
			x += fontWidthArray[(int)(*p)]/SPACE_KERNING;
		else
			x += fontWidthArray[(int)(*p)];
	return x;
	
	}


/**
 * Render text using the currently loaded font and currently set font size.
 * Rendering starts at coordinates (x, y), z is always 0.
 * The pixel coordinates that the FreeType2 library uses are scaled by (sx, sy).
 
 * MJS: I'm using the cached bitmaps and advance values generated by my font_init()
 */

void render_text(const char *text) {
	const char *p;
	int error;
	FT_GlyphSlot g = face->glyph;
	FT_BitmapGlyph bmg;
	FT_Glyph aglyf;
	float x=0;


	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);


	//FT_Set_Pixel_Sizes(face, 0, PIXEL_SIZE);
	glColor4f(1,1,1,1);


	/* Create a texture that will be used to hold one "glyph" */
	GLuint tex;

	glEnable(GL_MULTISAMPLE);
//	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE); // add this for gradual fading of label, but I don't like how grainy it becomes
	for (p = text; *p; p++) 
		{
		if (*p < MIN_ASCII || *p > MAX_ASCII)
				{
				printf("Invalid ascii character in text string\n");
				return;
				}
		bmg = glyf_bitmap[*p];
#if 0
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, bmg->bitmap.width, bmg->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, bmg->bitmap.buffer);
#endif
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); //necessary in ppm code, but maybe not here?


		if (*p != '_' && *p != ' ') // let's regard underscore as space and ignore either in trying to texture map; just nudge over 
			{
			glBindTexture(GL_TEXTURE_2D, fontTex[*p]);


		// do i need these here as they are being displayed? Possibly, or just when texture is being loaded?

				float x2 = x + bmg->left ;
				float y2 =  - bmg->top ;
				float w = bmg->bitmap.width ;
				float h = bmg->bitmap.rows ;
		// from renderPPM

				glBegin(GL_QUADS);
					glTexCoord2f(0,0);  glVertex3f(x2,-y2,0); 
					glTexCoord2f(1,0);  glVertex3f(x2 + w,-y2,0);
					glTexCoord2f(1,1);  glVertex3f(x2 + w,-y2 - h,0);
					glTexCoord2f(0,1);  glVertex3f(x2,-y2 - h,0);
				glEnd();
			x += fontWidthArray[(int)(*p)] ;
			}
		else
			x += fontWidthArray[(int)(*p)]/SPACE_KERNING;

		//y += (g->advance.y >> 6) * sy;
	}
//glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
glDisable(GL_MULTISAMPLE);
glDisable(GL_TEXTURE_2D);

glPopAttrib();
	return;
}
