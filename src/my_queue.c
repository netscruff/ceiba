#include <stdlib.h>
#include "my_queue.h"

queueStatic queueStaticNew(unsigned long size)
	{
	queueStatic Q;
	Q=(queueStatic)malloc(sizeof (struct queueStaticType));
	Q->first =(void **)malloc((size+1)*sizeof(void*));
	Q->head = Q->tail=Q->first;
	Q->last=Q->first+size;
	return Q;
	}

void enqueueStatic(queueStatic Q, void * e)
	{
	*(Q->tail) = e;
	if (Q->tail == Q->last)
		Q->tail=Q->first;
	else
		++(Q->tail);
	}
void * dequeueStatic(queueStatic Q)
	{
	void *x;
	if (Q->head == Q->tail)
		return NULL; 		// queue is empty
	x=*(Q->head);
	if (Q->head == Q->last)
		Q->head=Q->first;
	else
		++(Q->head);
	return x;
	}
void queueStaticFree(queueStatic Q)
	{
	free (Q->first);
	free (Q);
	}
#if 0
main()
	{
	int i;
	queueStatic Q;
	char *a="bob", *b="betty", *c="boop";
	Q=queueStaticNew(1000);
	for (i=1;i<1000;i++)
		enqueueStatic(Q,a);
	for (i=1;i<1000;i++)
		printf("%s\n",(char*)dequeueStatic(Q));
	queueStaticFree(Q);
	}
#endif
