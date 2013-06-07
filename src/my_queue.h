#ifndef _MY_QUEUE
#define _MY_QUEUE
typedef struct queueStaticType *queueStatic;
struct queueStaticType  
        {
        void**		head;  
	void**		tail;  
	void**		last;  
	void**		first; // array of the actual entries, these have pointers from first to last
        };

queueStatic queueStaticNew(unsigned long size);
void enqueueStatic(queueStatic Q, void * e);
void * dequeueStatic(queueStatic Q);
void queueStaticFree(queueStatic Q);

#include "my_slist.h"

#define queue	     slist
#define queueNew     slistNew
#define queueFree    slistFree
#define enqueue(a,b) slistInsert((a),(b))
#define dequeue(a)   slistPopLastElem(a)

#endif

