#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct _STL_Triangle {float n1,n2,n3,x1,y1,z1,x2,y2,z2,x3,y3,z3;char s[2];} __attribute__ ((packed)) STL_Triangle;

typedef struct _STL_Array {
	STL_Triangle *firstTriangle ; 
	long numTriangles ; 
	float xmin,xmax;
	float ymin,ymax;
	float zmin,zmax;
	} STL_Array;
/*

Some compilers, like gcc, by default pad their structure members to align them on boundaries, so that
if we assume they are packed tightly when reading a binary file, all hell could break loose.

The packed attribute above is understood by gcc and forces this problem away, although apparently it
can slow access.

*/



STL_Array * getSTLt(char * fileName);
STL_Array * getSTL(char * fileName);
void drawSTL(DrawEnum render, node a, enum LayoutChoice layout);

STL_Triangle *gTriangles;
long gNumTriangles;

