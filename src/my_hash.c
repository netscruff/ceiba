#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_hash.h"
#include "my_structures.h"

// Simple hash routine in which no deletions allowed!
// Keys are strings, and values may be any struct defined by user (see my_hash.h)

// *** Pay attention to the following ***

// The hashInsert function makes its own copy of the key and the pointer to the value. 
// Thus, we do not have to allocate a standalone key first. Presumably, the value had better be allocated somewhere first...
// We do this by storing a void pointer to the key and the value. 


// ***************************************

// All sizes  use unsigned longs, so sizes can be very large.

// NOTE! If you use integers as keys (i.e., as "strings"), the hash function doesn't always work very well,
// leaving lots of holes in the table and making lots of collisions. In general, should check performance
// of the string keys, or get smart and resize, etc.

Entry hashGetKthEntry(Hash h, long k) // get the kth entry where k = 0 ... numElements-1 ; return NULL if k too large
					// Note that the order of these entries might well change! It's a hash! This is just to permit slow
					// sequential access; the stuff that iterators are made of when its not a hack like this 
	{
	long i;
	Entry p;
	if (k >= h->numElements || k < 0)
		return NULL; 
        for (i=0;i<h->bSize;i++)
                {
                for (p=(Entry)vectorGet(h->b,i); p; p = p->next)
			if (k-- == 0)
				return p;
		}
	return NULL;
	}

Hash hashNew(unsigned long bSize) // this parameter sets the initial bucket count
	{
	Hash H;
	H=(Hash)malloc(sizeof(struct HashType));
	H->numElements=0;
	H->grow=1.6;
	H->b = (Vector)vectorNew(bSize);
	H->v = (Vector)vectorNew(bSize*H->grow); // allow more entries than buckets at first
	H->bSize=bSize;
	return H;
	}
void hashFree(Hash h)
	{
	vectorFree(h->v);
	vectorFree(h->b);
	free (h);
	}
void hashPrint(Hash h)
	{
	long ix;
	Entry p;
	printf("Printing keys in hash table...\n");
	for (ix=0;ix<h->bSize;ix++)
		{
		printf("Hash table index: %li\n",ix);
		for (p=(Entry)vectorGet(h->b,ix); p; p = p->next)
			entryPrint(p);	
		}
	return;
	}
void  entryPrint(Entry e) // print the key only...
	{
	if (e)
		//printf("%s\t%li\n",e->key,*(unsigned long *)(e->val));
		printf("%s\n",e->key);
	else
		printf("No entry present\n");
	return;
	}
Entry entryNew(char *key, void * val) // makes an entry, including a *copy* of the key (!), and the pointer to val
	{
	Entry e;
	e = (Entry)malloc(sizeof(struct EntryType));
	e->next=NULL;
	e->val=val;
	e->key=DupStr(key);
	return e;
	} 
Entry hashKeyExists(Hash h, char *key) // returns either a pointer to the entry or NULL if key is not present
	{
	unsigned long ixh;
	Entry p;
	ixh = hashFunc(key)%h->bSize; // the remainder after dividing the hash value by size of b array gives an index on 
				   // [0..bSize-2]
	for (p=(Entry)vectorGet(h->b,ixh); p; p = p->next) // should just bail if no entry has previously been inserted
		if (!strcmp(key,p->key))
			return p;
	return NULL;
	}

int hashInsert(Hash h, char *key, void * val, Entry *e)  
	// If key does not exist, "insert", return 1, and set e to inserted entry 
	// If key exists, "do not insert", return 0 and set e to found entry
	// ** Note ** we can pass a temporary copy of key; insert makes a new copy of it!
	{
	unsigned long ixh;
	Entry p,newp,pprev;
	// printf (" Attempting to insert %s %s\n",key, (char *)val);
	ixh = hashFunc(key)%h->bSize;
		 // the remainder after dividing the hash value by size of b array gives an index on [0..bSize-2]
	p=(Entry)vectorGet(h->b,ixh);
	if (p==NULL) // no entry at this index in hash table, must insert obviously, then skip to finish up
		{
		newp=entryNew(key,val);
		vectorInsertAt(h->b,ixh,newp);// store the pointer in bucket
		}
	else //collision, either find match, or failing that, get last entry, pprev, and insert new entry after it
		{
		for (; p; pprev=p,p = p->next) // should just bail if no entry has previously been inserted
			{
			if (!strcmp(key,p->key))
				{
				*e = p;
				return 0;  // found key, so return pointer to entry
				}
			}
		newp=entryNew(key,val);
		pprev->next=newp;
		}
	vectorPushBack(h->v,newp);  // store the pointer in v array
	++(h->numElements);

//	if ( h->v->numElements  >  h->b->capacity ) // departure from usual: I just resize under these conditions
	if ( h->v->top  >  h->b->capacity ) // departure from usual: I just resize under these conditions
		hashResize (h);
	*e=newp;
	return 1; // key not found; so we insert and return 1
	} 

void hashResize(Hash h) // increases the number of buckets and then rehashes everything 
	{
	unsigned long newsz,i,ixh,lastEntry,numBuckets;
	Entry e;
	numBuckets=(h->b)->capacity;
	lastEntry = h->v->top -1;
	newsz = numBuckets * h->grow;
	vectorReserve(h->b,newsz);
	h->bSize=newsz;
	for (i=0;i<newsz;i++)
		vectorInsertAt(h->b,i,NULL);

	// rehash, etc.

	for (i=0;i<=lastEntry;i++)
		{
		e = vectorGet(h->v,i);
		ixh = hashFunc(e->key) % newsz;
		e->next = vectorGet(h->b,ixh);
		vectorInsertAt(h->b,ixh,e);
		}
	}

unsigned long hashFunc(char *key) // ripped off from B.Stroustrup, The C++ Programming Language, 3rd ed., 1997.
	{
	unsigned long res=0,len;
	char *p;
	p=key;
	len = strlen(key);
	while (len--) res = (res<<1)^*p++;
	return res;
	}
unsigned long hashNumIntersect(Hash h1, Hash h2) // rough implementation to find the number of hash keys common to two hashes (ie. the intersection)
	{
	unsigned long ins=0,lastEntry,i;
	Entry e;
	lastEntry = h1->v->top -1;
	for (i=0;i<=lastEntry;i++)
		{
		e = vectorGet(h1->v,i);
		if ( hashKeyExists(h2,e->key))
			++ins;
		}

	return ins;
	}                

#if 0

main(int argc,char * argv[])  // currently this main tests the hash intersection feature
{
char fnInput[64], key[128], key2[128];
FILE * inStream =NULL;
unsigned long line = 0,i,hashSize=100,val,*lv;
Hash h,h1,h2;
Entry e;
strcpy(fnInput,argv[1]);
inStream=fopen(fnInput,"r");

h = hashNew(hashSize);
h1 = hashNew(hashSize);
h2 = hashNew(hashSize);

while (!feof(inStream))
	{
	fscanf(inStream,"%s %s",&key,&key2);
	++line;
	hashInsert(h1,key,NULL,&e);
	hashInsert(h2,key2,NULL,&e);
	}
// printf("Number of keys in hash:%li\n",h->numElements);
// printf("Size of bucket array:%li\n",(h->b)->capacity);

printf("Size of intersection = %li\n",hashNumIntersect(h1,h2));

hashFree(h1);
hashFree(h2);

//for (i=0;i<(h->v)->numElements;i++)
//	printf ("%i\t%s\n",i,((Entry)vectorGet(h->v,i))->key);

}

#endif
