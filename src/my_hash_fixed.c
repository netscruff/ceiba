#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_hash_fixed.h"
#include "my_structures.h"

// Simple hash routine in which size of hash table is fixed and no deletions allowed!
// Keys are strings, and values may be any struct defined by user (see my_hash.h)

// All structures use unsigned longs, so sizes can be very large.

// NOTE! If you use integers as keys (i.e., as "strings"), the hash function doesn't always work very well,
// leaving lots of holes in the table and making lots of collisions. In general, should check performance
// of the string keys, or get smart and resize, etc.



Hash newHash(unsigned long size)
	{
	Hash H;
	H=(Hash)malloc(sizeof(struct HashType));
	H->numElements=0;
	H->b = (Entry*)calloc(size*sizeof (Entry),sizeof(Entry)); // initialize to nulls
	H->bSize=size;
	return H;
	}
void printHash(Hash h)
	{
	long ix;
	Entry p;
	printf("Printing entries in hash table...\n");
	for (ix=0;ix<h->bSize;ix++)
		{
		printf("Hash table index: %li\n",ix);
		for (p=(h->b)[ix]; p; p = p->next)
			printEntry(p);	
		}
	return;
	}
void  printEntry(Entry e) // print the key only...
	{
	if (e)
		//printf("%s\t%li\n",e->key,*(unsigned long *)(e->val));
		printf("%s\n",e->key);
	else
		printf("No entry present\n");
	return;
	}
Entry newEntry(char *key, valStruct val)
	{
	Entry e;
	e = (Entry)malloc(sizeof(struct EntryType));
	e->next=NULL;
	e->val=val;
	e->key=DupStr(key);
	return e;
	} 
Entry exists(Hash h, char *key) // returns either a pointer to the entry or NULL if key is not present
	{
	unsigned long ixh;
	Entry p;
	ixh = hash(key)%h->bSize; // the remainder after dividing the hash value by size of b array gives an index on 
				   // [0..bSize-2]
	for (p=(h->b)[ixh]; p; p = p->next) // should just bail if no entry has previously been inserted
		if (!strcmp(key,p->key))
			return p;
	return NULL;
	}

valStruct insert(Hash h, char *key, valStruct val)  
	// If key exists, return pointer to val; if not, insert key/val and return NULL (!).
	//  This is odd but useful! 
	{
	unsigned long ixh;
	Entry p,newp,pprev;
	ixh = hash(key)%h->bSize;
		 // the remainder after dividing the hash value by size of b array gives an index on [0..bSize-2]
	p=(h->b)[ixh];
	if (p==NULL) // no entry at this index in hash table, must insert obviously, then skip to finish up
		{
		newp=newEntry(key,val);
		(h->b)[ixh]=newp;
		}
	else //collision, either find match, or failing that, get last entry, pprev, and insert new entry after it
		{
		for (; p; pprev=p,p = p->next) // should just bail if no entry has previously been inserted
			{
			if (!strcmp(key,p->key))
				return p->val;  // found key, so return pointer to val
			}
		newp=newEntry(key,val);
		pprev->next=newp;
		}
	++(h->numElements);
	return NULL; // key not found; so we insert and return null
	} 

unsigned long hash(char *key) // ripped off from B.Stroustrup, The C++ Programming Language, 3rd ed., 1997.
	{
	unsigned long res=0,len;
	char *p;
	p=key;
	len = strlen(key);
	while (len--) res = (res<<1)^*p++;
// printf("%s\t%li\n",key,res);
	return res;
	}
                
#if 0

main(int argc,char * argv[])
{
char fnInput[64], key[128];
FILE * inStream =NULL;
unsigned long line = 0,hashSize=100,val,*lv;
Hash h;

strcpy(fnInput,argv[1]);
inStream=fopen(fnInput,"r");
hashSize = strtol(argv[2],NULL,0);
h = newHash(hashSize);

while (!feof(inStream))
	{
	fscanf(inStream,"%s %li",&key,&val);
	++line;
	lv = (unsigned long *)malloc(sizeof(unsigned long));
	*lv=val;
	insert(h,key,lv);
	}
//printHash(h);
printf("Number of keys in hash:%li\n",h->numElements);
printf("Size of hash:%li\n",h->bSize);
}

#endif
