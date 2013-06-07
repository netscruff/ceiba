#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct _XYZ_atom {char label; float x,y,z;} XYZ_atom;
typedef struct _XYZ_bond {int i,j;} XYZ_bond;

typedef struct _XYZ_Array {
	XYZ_atom *firstAtom ; 
	XYZ_bond *firstBond;
	int numAtoms ; 
	int numBonds ;
	float xmin,xmax;
	float ymin,ymax;
	float zmin,zmax;
	} XYZ_Array;
/*

Some compilers, like gcc, by default pad their structure members to align them on boundaries, so that
if we assume they are packed tightly when reading a binary file, all hell could break loose.

The packed attribute above is understood by gcc and forces this problem away, although apparently it
can slow access.

*/



XYZ_Array * getXYZ(char * fileName);
XYZ_Array * getXYZ_v2(char * fileName);
void drawXYZ(DrawEnum render, node a, enum LayoutChoice layout);
int split(char *s, char* delimiters, char* words[]);


