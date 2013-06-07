
#include "my_slist.h"
#include <stdlib.h>
#include <stdio.h>


/* Singly linked list structure

The list maintains pointers to objects--it does *not* copy the objects themselves and store them!
That means you better have them allocated first.

This structure can be used as a queue by using slistInsert to 'enqueue' and slistPopLastElem to 'dequeue',
because the first call inserts at the head of the list, while the last one pops off the tail of the list.

*/


slistEntry slistEntryNew(void * elem) // creates a new entry with a pointer to elem; entry is hanging in limbo
	{
	slistEntry e;
	e = (slistEntry)malloc(sizeof (struct slistEntryType));
	e->elem = elem;
	e->next = NULL;
	return e;
	}
slist slistNew(void)
	{
	slist sl;
	sl = (slist)malloc(sizeof (struct slistType));
	sl->numElements=0;
	sl->head = NULL; 
	return sl;
	}
void slistInsert(slist S, void * elem) // inserts a new entry at head of list; entry contains a pointer to elem
	{
	slistEntry e;
	e = slistEntryNew(elem);
	e->next = S->head;
	S->head = e;
	++S->numElements;
	}
void slistDelete(slist S, void * elem) // deletes entry in the list that contains the pointer to *elem.
					// Careful to remember that the equality test here is just at the level of the pointer to the same object
	{
	slistEntry e,prev;
	if (S==NULL || S->numElements==0) return; // no action
	prev=NULL;
	for (e=S->head; e ; prev=e, e=e->next)
		{
		if (elem == e->elem)
			{
			if (prev==NULL)
				S->head = e->next; //deleted entry was first in list
			else
				prev->next = e->next; // deleted entry was in middle or end of list
			free(e);
			--S->numElements;
			return;	
			}
		}
	}
void * slistPopLastElem(slist S)
	{
	slistEntry e,prev;
	void * elem;
	if (S==NULL || S->numElements==0) return NULL; // no action
	e=S->head;
	prev=NULL;
	while (e->next) // find the last entry in list
		{
		prev=e;
		e=e->next;
		}
	if (prev==NULL)
		S->head=NULL;
	else
		prev->next=NULL; // remove link to last element
	elem = e->elem;
	free (e);
	--S->numElements;
	return elem; 
	}
void slistFree(slist S)
	{
	slistEntry e,prev;
	if (S)
		{
		e=S->head;
		if (e) // there's at least one element; have to free these before freeing list structure itself
			do	
				{
				prev=e;
				e=e->next;
				free (prev);
				}
			while (e);
		free (S);
		}
	else
		printf("Can't free nonexistent list\n");
	}
#if 0
#include <string.h>
main ()
	{
	char *a,*b,*c, *s;
	slist S;
	slistEntry e;
	a=(char*)malloc(10*sizeof (char));
	b=(char*)malloc(10*sizeof (char));
	c=(char*)malloc(10*sizeof (char));
	strcpy(a,"bob");
	strcpy(b,"betty");
	strcpy(c,"boop");

	S = slistNew();
	slistInsert(S,a);
	slistInsert(S,b);
	slistInsert(S,c);
	for (e=S->head;e !=NULL; e=e->next)
		printf ("%s\n",e->elem);

	printf("\n");
	slistDelete(S,a);
	for (e=S->head;e !=NULL; e=e->next)
		printf ("%s\n",e->elem);
	slistInsert(S,a);

	printf("\n");
	slistDelete(S,b);
	for (e=S->head;e !=NULL; e=e->next)
		printf ("%s\n",e->elem);
	slistInsert(S,b);

	printf("\n");
	slistDelete(S,c);
	for (e=S->head;e !=NULL; e=e->next)
		printf ("%s\n",e->elem);
	slistInsert(S,c);
/*
	s= (char*)slistPopLastElem(S);
	printf("Popped item = %s\n",s);
	s= (char*)slistPopLastElem(S);
	printf("Popped item = %s\n",s);
	s= (char*)slistPopLastElem(S);
	printf("Popped item = %s\n",s);
*/
	slistFree(S);
	}
#endif
