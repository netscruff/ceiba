#ifndef _MY_SLIST
#define _MY_SLIST
typedef struct slistType *slist;
typedef struct slistEntryType *slistEntry;
struct slistType  
        {
        slistEntry 	head;  
        unsigned long 	numElements;
        };

struct slistEntryType
	{
	void * 		elem;
	slistEntry 	next;
	}; 

slistEntry slistEntryNew(void * elem);
slist slistNew(void);
void * slistPopLastElem(slist S);
void slistInsert(slist S, void * elem);
void slistDelete(slist S, void * elem);
void slistFree(slist S);
#endif
