#ifndef _MY_HASH
#define _MY_HASH
#include "my_vector.h"




// ********************

typedef struct EntryType *Entry;
struct EntryType 
	{
	char * key;
	Entry  next;
	void * val;
	}; 

typedef struct HashType *Hash;
struct HashType
	{
	Vector v; 		// array of pointers to the elements
	Vector b;		// array of pointers to the buckets
	unsigned long bSize; 	// number of buckets (size of the hash table)(space in b) 
	unsigned long numElements;  // current number of entries
	float grow;		// when necessary grow the number of buckets by this factor
	};



Entry hashGetKthEntry(Hash h, long k); 
void 	hashResize(Hash h);
int 	hashInsert(Hash h, char *key, void * val, Entry *e);
void 	hashPrint(Hash h);
void 	hashFree(Hash h);
Hash 	hashNew(unsigned long size);
Entry 	hashKeyExists(Hash h, char *Key);

unsigned long hashNumIntersect(Hash h1, Hash h2);
unsigned long hashFunc(char *key);

void  entryPrint(Entry e);
Entry entryNew(char *key, void * val);

#endif
