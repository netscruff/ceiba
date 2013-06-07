#ifndef _MY_VECTOR
#define _MY_VECTOR
// *** The following defines the data structure for the values stored in the vector 

typedef void * vecValStruct; // Can store pointers to anything... 


// ********************

typedef struct VectorType *Vector;
struct VectorType 
	{
	vecValStruct *elem; // the actual array of values
	unsigned long capacity; // current memory allocation
	unsigned long top; // address of first free space at top of vector ; also effectively the number of elements in array
				// neglecting any holes that might be present in the array
	}; 

unsigned long vectorLastIndex(Vector v);
void vectorFree(Vector v);
void vectorPushBack(Vector V, vecValStruct val);
void vectorInsertAt(Vector V, unsigned long ix, vecValStruct val);
vecValStruct vectorGet(Vector V, unsigned long ix);
Vector vectorNew(unsigned long size);
void vectorGrow(Vector v);
void vectorReserve(Vector V,unsigned long newSize);
unsigned long vectorCapacity(Vector v);
unsigned long vectorNumElements(Vector v);
#endif
