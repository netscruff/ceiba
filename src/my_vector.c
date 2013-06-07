#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_vector.h"
#include "my_structures.h"

void vectorFree(Vector v)
	{
	free (v->elem); // free the array
	free (v);
	}

unsigned long vectorCapacity(Vector v)
	{
	return v->capacity;
	}
unsigned long vectorLastIndex(Vector v)
	{
	return v->top-1;
	}
unsigned long vectorNumElements(Vector v)
	{
	return v->top;
	}
Vector vectorNew(unsigned long sz)
	{
	Vector V;
	V=(Vector)malloc(sizeof(struct VectorType));
	V->capacity=sz;
	V->top = 0; 
	V->elem = (vecValStruct*)calloc(sz,sizeof(vecValStruct)); 
	return V;
	}
vecValStruct vectorGet(Vector v, unsigned long ix) 
	{
	return (v->elem)[ix];
	}
void vectorInsertAt(Vector v, unsigned long ix, vecValStruct val) 
	// replaces the pointer-element at ix with val (no range checking) 
	// Note, I can't think of a reliable way to check the number of elements, since there may be holes in the array.
	// The code below for numElements is unreliable, unless we can always initialize all space to NULLs
	{
	unsigned long top;
	if (ix >= v->capacity)
		vectorReserve(v,ix+1); // this allows inserting something out of bounds on the high end!
	(v->elem)[ix] = val;
	if (ix >= v->top)
		v->top = ix+1;		// check to see if this new element is the top one.
	return;
	}
void vectorPushBack(Vector v, vecValStruct new)
	{
	unsigned long top;
	top=v->top;
	if (top >= v->capacity)
		vectorGrow(v);
	(v->elem)[top] = new;
	++v->top;	
	return;
	}
#define GROW_VECTOR 1.6
void vectorReserve(Vector V,unsigned long newSize)
	{
	vecValStruct *newElemArray;
	newElemArray=realloc(V->elem, newSize*sizeof(vecValStruct));
	if (newElemArray)
		{
		V->elem=newElemArray;
		V->capacity=newSize;
		}
	else
		printf ("Failure to reallocate my_vector\n");
	}
void vectorGrow(Vector V)
	{
	vectorReserve(V,V->capacity*GROW_VECTOR);
	}


#if 0

main(int argc,char * argv[])
{
char fnInput[64], key[128], val[128];
FILE * inStream =NULL;
unsigned long line = 0,vSize=100,i;
Vector V;

strcpy(fnInput,argv[1]);
inStream=fopen(fnInput,"r");
V = vectorNew(vSize);

while (EOF != fscanf(inStream,"%s %s",&key,&val))
		vectorPushBack(V,DupStr(key));

printf("Number of elements in vector:%li\n",V->numElements);
printf("Size of vector:%li\n",V->capacity);

vectorInsertAt(V,1500000,DupStr(key));

printf("Number of elements in vector:%li\n",V->numElements);
printf("Size of vector:%li\n",V->capacity);

printf("%s\n",vectorGet(V,1500000));

/*for (i=0;i<V->top;i++)
	printf("%li:%s\n",i,(V->elem)[i]);
*/
}
#endif
