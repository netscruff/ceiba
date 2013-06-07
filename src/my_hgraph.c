#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_slist.h"
#include "my_hash.h"
#include "my_hgraph.h"
#include "my_queue.h"
#include "my_structures.h"

int gPrintFlag;
static void hgSEWhelper(vertex v1, vertex v2, long wt);
static void BFSRemoveBorderEdges(hgraph G, vertex s);

Neighbor neighborNew(vertex v, long w)
	{
	Neighbor c;
	c = (Neighbor)malloc(sizeof(struct neighborStruct));
	c->v = v;
	c->weight = w;
	return c;
	}
Component componentNew(vertex v, unsigned long size)
	{
	Component c;
	c = (Component)malloc(sizeof(struct compStruct));
	c->v = v;
	c->size = size;
	return c;
	}
void vertexFree(vertex v)
	{
	slistFree(v->adj);
	free(v->key);
	free(v);
	}
#if BIPARTITE
vertex vertexNew(char * key, enum sideChoice side)  // stores a copy of the key 
#else
vertex vertexNew(char * key)  // stores a copy of the key 
#endif
	{
	vertex v;
	v= (vertex)malloc(sizeof (struct vertexStruct));
	v->key = DupStr(key);
	v->compNum=0;
	v->color=white;
	v->adj = slistNew();
#if BIPARTITE
	v->side = side;
#endif
	return v;
	}

Entry hgraphVertexKeyExists(hgraph g, char * key)
	{
	return hashKeyExists(g, key); 
	}
hgraph hgraphNew(void)
	{
	return hashNew(INIT_BUCKETS);
	}
int hgraphAddVertex(hgraph hg, vertex v) // the key is copied via vertexNew (for the val) and hashInsert (for the key)
	{
	Entry e;
	return  hashInsert(hg,v->key,v,&e); // insert vertex unless it's already there...return 1 if insert occurred
	}
#if 0
void hgraphAddEdgeVx(hgraph hg, vertex v1, vertex v2)
	{
	if (!v1 || !v2) 
		printf ("Add edge failed because one or both vertices not in graph\n");
	if (strcmp(v1->key,v2->key))
		{
		slistInsert(v1->adj,v2); // add each vertex to the other's adjacency list
		slistInsert(v2->adj,v1);
		}
	else
		slistInsert(v1->adj,v1); //they're the same; just add one to adjacency list
	}
#endif
void hgraphAddEdge(hgraph hg, char *a, char *b,long weight)
	{
	vertex v1, v2;
	v1=hashKeyExists(hg,a)->val;
	v2=hashKeyExists(hg,b)->val;
	hgraphAddEdgeVx(hg,v1,v2,weight);
	}
void hgraphAddEdgeVx(hgraph hg, vertex v1, vertex v2, long weight)
	{
	Neighbor n1,n2;
	if (!v1 || !v2) 
		printf ("Add edge failed because one or both vertices not in graph\n");
	if (v1 != v2)
		{
		n1=neighborNew(v1,weight);
		n2=neighborNew(v2,weight);
		slistInsert(v1->adj,n2); // add each vertex neighbor to the other's adjacency list
		slistInsert(v2->adj,n1);
		}
	else
		{
		n1=neighborNew(v1,weight);
		slistInsert(v1->adj,n1); //they're the same; just add one to adjacency list
		}
	}
void hgraphDeleteEdge(vertex v1, vertex v2) 
	{
	hgraphDeleteNeighbor(v1,v2);
	hgraphDeleteNeighbor(v2,v1);
	return;
	}
void hgraphDeleteNeighbor(vertex v, vertex n) 
	{
	slist S;
	slistEntry e,prev;
	if (!v || !n) printf("Delete neighbor failed because one or both vertices not in graph\n");
	S=v->adj;
	if (S==NULL || S->numElements==0) return; // no action
	prev=NULL;
	for (e=S->head; e ; prev=e, e=e->next)
		{
		if (((Neighbor)(e->elem))->v == n)
			{
			if (prev==NULL)
				S->head = e->next; //deleted entry was first in list
			else
				prev->next = e->next; // deleted entry was in middle or end of list
			free(e->elem);
			free(e);
			--S->numElements;
			return;	
			}
		}
	}
long hgraphEdgeWeight(vertex v1, vertex v2) 
	{
	slist S;
	slistEntry e;
	Neighbor nb;
	if (!v1 || !v2) printf("EdgeWeight failed because one or both vertices not in graph\n");
	S=v1->adj;
	if (S==NULL || S->numElements==0) return; // no action
	for (e=S->head; e ; e=e->next)
		{
		nb=(Neighbor)(e->elem);
		if (nb->v == v2)
			{
			return nb->weight;	
			}
		}
	printf("Couldn't find edge weight\n");
	return 0;
	}
void hgraphSetEdgeWeight(vertex v1, vertex v2,long wt) 
	{
	hgSEWhelper(v1,v2,wt);
	hgSEWhelper(v2,v1,wt);
	}
static void hgSEWhelper(vertex v1, vertex v2, long wt)
	{
	slist S;
	slistEntry e;
	Neighbor nb;
	if (!v1 || !v2) printf("SetEdgeWeight failed because one or both vertices not in graph\n");
	S=v1->adj;
	if (S==NULL || S->numElements==0) return; // no action
	for (e=S->head; e ; e=e->next)
		{
		nb=(Neighbor)(e->elem);
		if (nb->v == v2)
			{
			nb->weight = wt;	
			return;
			}
		}
	printf("Couldn't find edge \n");
	return ;
	}
#if BIPARTITE
void   hgraphNumLRVertices(hgraph g, unsigned long *L, unsigned long *R)
	{
	unsigned long ix;
	Entry e;
	vertex u;
	*L=*R=0;
	for (ix = 0; ix<g->numElements; ix++)
		{
		e = (Entry)vectorGet(g->v,ix);
		u = (vertex)(e->val); 
		if (u->side == left)
			++*L;
		else
			++*R;
		}
	}
#endif

Vector components(hgraph g, int printFlag) // based on Cormen et al. (Introduction to algorithms, 2nd ed.), p. 541
			// ...it works (compared it to my BLINK algorithm on two large data sets)


	{
	unsigned long ix,c=0,numV;
	Entry e;
	vertex u;
	Vector compVec;
	Component comp;
	gPrintFlag=printFlag;
	compVec=vectorNew(100);
	for (ix = 0; ix<g->numElements; ix++)
		{
		e = (Entry)vectorGet(g->v,ix);
		u = (vertex)(e->val); 
		u->color = white;
		}
	for (ix = 0; ix<g->numElements; ix++)
		{
		e = (Entry)vectorGet(g->v,ix);
		u = (vertex)(e->val); 
		if (u->color == white)
			{
			numV=DFS(u,c); // choose this one for speed, the next one if you want more control over components
//			numV=cutoffDFS(u,c,0);
//printf ("Component %li size %li\n",c,numV);
			comp = componentNew(u,numV);
			vectorInsertAt(compVec,c, comp);
			++c; // component number
			}
		}
	return compVec;
	}

unsigned long  DFS(vertex u, unsigned long c) // returns number of vertices visited in this component
	{
	vertex v;
	unsigned long numV=1;
	Neighbor n;
	slistEntry se;
	u->color = gray;
	u->compNum=c;
#if BIPARTITE
	if (u->side == right)  // for plink use, we just want to print nodes on the right side of bipart graph
#endif
	if (gPrintFlag)
		printf ("%li %s\n",c,u->key);
	for (se=u->adj->head; se != NULL; se=se->next)
			{
			n = (Neighbor)(se->elem);
			v = (vertex)(n->v);
			if (v->color == white)
				numV+=DFS(v,c);
			}
	u->color=black;
	return numV;	
	}
unsigned long  cutoffDFS(vertex u, unsigned long c, int cutoff) // returns number of vertices visited in this component but if any edge weight
								// is < cutoff, the edge is "removed" from consideration
        {
        vertex v;
	Neighbor n;
        unsigned long numV=1,wt;
        slistEntry se;
        u->color = gray;
        u->compNum=c;
        printf ("%li %s\n",c,u->key);
        for (se=u->adj->head; se != NULL; se=se->next)
                        {
			n = (Neighbor)(se->elem);
			v = (vertex)(n->v);
			wt = n-> weight;
                        if (v->color == white && wt >= cutoff)
                                numV+=cutoffDFS(v,c,cutoff);
                        }
        u->color=black;
        return numV;
        }
unsigned long BFS(hgraph G, vertex s)  // from Cormen et al. p. 532
			
        {
        vertex u,v;
	Neighbor n;
        unsigned long ix, count=0;
	Entry e;
        slistEntry se;
	queue Q;
	Q=queueNew();
	for (ix = 0; ix<G->numElements; ix++)
		{
		e = (Entry)vectorGet(G->v,ix);
		u = (vertex)(e->val); 
		u->color = white;
		}
        s->color = gray;
	enqueue(Q,s);
	while (u=dequeue(Q)) // terminate if this is NULL, meaning the queue is empty
		{
        	for (se=u->adj->head; se != NULL; se=se->next)
                        {
			n = (Neighbor)(se->elem);
			v = (vertex)(n->v);
                        if (v->color == white)
				{
				v->color=gray;
				enqueue(Q,v);
				}
                        }
        //	printf ("Visiting vertex %s\n",u->key);
	        u->color=black;
		++count;
		}
	return count;
        }

unsigned long BFSlimit(hgraph G, vertex s, unsigned long N)  // from Cormen et al. p. 532
			
        {
        vertex u,v;
	Neighbor n;
        unsigned long ix, count=0;
	Entry e;
        slistEntry se;
	queue Q;
	Q=queueNew();
	for (ix = 0; ix<G->numElements; ix++)
		{
		e = (Entry)vectorGet(G->v,ix);
		u = (vertex)(e->val); 
		u->color = white;
		}
        s->color = gray;
	enqueue(Q,s);
	while (u=dequeue(Q)) // terminate if this is NULL, meaning the queue is empty
		{
		if (count >= N) return count;
        	for (se=u->adj->head; se != NULL; se=se->next)
                        {
			n = (Neighbor)(se->elem);
			v = (vertex)(n->v);
                        if (v->color == white)
				{
				v->color=gray;
				enqueue(Q,v);
				}
                        }
        	printf ("Visiting vertex %s\n",u->key);
	        u->color=black;
		++count;

		u->compNum=1;
		}
	return count;
        }

unsigned long BFSmatch(hgraph G, vertex s, long matchCode)  // from Cormen et al. p. 532
	
// traverses those elements of the graph that have vx->comNum == matchCode, and counts these nodes -- stays within the component
// be sure to initialize the vertex field value before running. Have to start at vertex s within the component of interest.
	
        {
        vertex u,v;
	Neighbor n;
        unsigned long ix, count=0;
	Entry e;
        slistEntry se;
	queueStatic Q;
	Q=queueStaticNew(G->numElements);
	for (ix = 0; ix<G->numElements; ix++) // oops careful here! This will whiten the whole graph again, messing up any other BFS
		{
		e = (Entry)vectorGet(G->v,ix);
		u = (vertex)(e->val); 
		u->color = white;
		}
        s->color = gray;
	if (s->compNum == matchCode)
		enqueueStatic(Q,s);
	while (u=dequeueStatic(Q)) // terminate if this is NULL, meaning the queue is empty
		{
        	for (se=u->adj->head; se != NULL; se=se->next)
                        {
			n = (Neighbor)(se->elem);
			v = (vertex)(n->v);
                        if (v->color == white && v->compNum==matchCode)
				{
				v->color=gray;
				enqueueStatic(Q,v);
				}
                        }
//        	printf ("Visiting vertex %s\n",u->key);
	        u->color=black;
		++count;
		}
	queueStaticFree(Q);
	return count;
        }
int isAdjacent(vertex u, long matchCode)

// is there at least one vertex adjacent to v that has its compNode field == matchCode ?

	{
	Neighbor n;
	vertex v;
	slistEntry se;
        for (se=u->adj->head; se != NULL; se=se->next)
		{
		n = (Neighbor)(se->elem);
		v = (vertex)(n->v);
		if (v->compNum == matchCode) 
			return 1;
		}
	return 0;	
	}

int isDisconnectVertex(hgraph G, vertex s)
	{
	// ** Under construction
	// is the component containing s disconnected if s is removed ?
	int isDis = 0;
	long nB,C_size;
        vertex u,ve,s_next;
	queueStatic Q;
	Q=BFSQueue(G,s); 
	C_size=BFS(G,s); // The size of this component
	if (C_size <= 2)
		return 0; // can't be disconnecting
	u=dequeueStatic(Q);
	u->compNum=1; // this will be vertex s
	u=dequeueStatic(Q);
	u->compNum=0;
	s_next=u; 
	while (u=dequeueStatic(Q))
		u->compNum=0;
	queueStaticFree(Q);
	nB = BFSmatch(G,s_next, 0);
	if (nB == C_size -1)
		return 0; // remains a component
	else
		return 1;
	}
vertex splitGraph(hgraph G, vertex s, unsigned long A_size)  

	// ** Under construction

// Partition a graph into an A element and B element, each of which is a connected component. Vertex s is any vertex in the graph G.
// Returns a vertex in the B element; s will be the first vertex in the A element. The two components will have the edges that formerly
// joined them removed.	
        {
        vertex u,u_candidate,u_next,v,B_save;
	Neighbor n;
        unsigned long ix, numA, numB,count=0,nB, G_size;
	Entry e;
        slistEntry se;
	queueStatic Q;
	for (ix = 0; ix<G->numElements; ix++) // just init this piece of data in all nodes
		{
		e = (Entry)vectorGet(G->v,ix);
		u = (vertex)(e->val); 
		u->compNum = 0;
		}
	Q=BFSQueue(G,s); // set up a queue that has all the nodes in a BFS traversal order (in this component, of course)
			 // ...notice that this will deliver s as the first node in the queue inthe next statement	
	G_size=BFS(G,s); // The size of this component
	if (G_size < 2)
		printf("Graph must have at least two vertices to split\n");
	if (G_size < A_size)
		printf ("Requested size of partition element is larger than the graph component\n");
	u=dequeueStatic(Q);  // u is a node in A; u_candidate is not in A, but we are trying it out; u_next is a neighbor of u_candidate not in A
	u->compNum=1;
	numA=1; 	// this is the first node of part A

	u_candidate=dequeueStatic(Q);  // careful! This might not be adjacent to any of the elements of A, in particular if there is a bridge
					// between A and the node that just failed ...need to check whether candidate is adjacent


	while (u_next=dequeueStatic(Q))  // note that all we need is some vertex in B, might as well use the next one in the Q
		{
		if (numA < A_size)  // keep chunking it until A is big enough
			{
printf ("u_candidate=%s u_next=%s numA=%li\n",u_candidate->key,u_next->key,numA);
			if (!isAdjacent(u_candidate,1))
				{
				printf("A candidate -- %s -- was not adjacent!!\n",u_candidate->key);
				}
			else
				{
				u_candidate->compNum=1; // temporarily set this to 1
				nB = BFSmatch(G,u_next, 0);
				if ( nB  == G_size - numA -1  ) // ...if u_next (which is just a neighbor of u, not already in A)
								       // is in a component of size comprising *all* the rest of graph!
								      // ...which we detect by leaving the vx code = 0 in that part of graph
					++numA;				  // ...then its OK to add node u to A  
				else
					{ 				// it's not OK to add this, instead it remains in part B 
					u_candidate->compNum=0;  // reset it
					enqueueStatic(Q,u_candidate);
					}
				// ... otherwise try the next candidate	
				u_candidate=u_next;
				}
			}
		else
			break;	
		}
	if (numA < A_size)
		printf("Failed to construct component of required size\n");
	queueStaticFree(Q);
	BFSRemoveBorderEdges(G,s);
	return u_candidate;  // this should always be a vertex in B... 
        }

queueStatic BFSQueue(hgraph G,vertex s)  // from Cormen et al. p. 532

// traverses a graph via BFS and stores all the vertices in a new queue in that traversal order.
// Rather wastefully insantiates a static queue the size of the whole graph, rather than the size of the component, oh well	
        {
        vertex u,v;
	Neighbor n;
        unsigned long ix;
	Entry e;
        slistEntry se;
	queueStatic Q, retQ;
	Q=queueStaticNew(G->numElements);
	retQ=queueStaticNew(G->numElements);
	for (ix = 0; ix<G->numElements; ix++)
		{
		e = (Entry)vectorGet(G->v,ix);
		u = (vertex)(e->val); 
		u->color = white;
		}
        s->color = gray;
	enqueueStatic(Q,s);
	while (u=dequeueStatic(Q)) // terminate if this is NULL, meaning the queue is empty
		{
        	for (se=u->adj->head; se != NULL; se=se->next)
                        {
			n = (Neighbor)(se->elem);
			v = (vertex)(n->v);
                        if (v->color == white)
				{
				v->color=gray;
				enqueueStatic(Q,v);
				}
                        }
	        u->color=black;
		enqueueStatic(retQ,u);
		}
	queueStaticFree(Q);
	return retQ;
        }
static void BFSRemoveBorderEdges(hgraph G, vertex s)
	
        {
        vertex u,v;
	Neighbor n;
        unsigned long ix;
	Entry e;
        slistEntry se,se_next;
	for (ix = 0; ix<G->numElements; ix++) 
		{
		e = (Entry)vectorGet(G->v,ix);
		u = (vertex)(e->val); 
		if (u->compNum==1)
		    {
		    se=u->adj->head;
        	    while (se != NULL) // had trouble originally using a for() here, but we delete elements out of the middle of the for() statement
                        {
			se_next=se->next;
			n = (Neighbor)(se->elem);
			v = (vertex)(n->v);
			if (v->compNum==0)
				hgraphDeleteEdge(u,v);
			se=se_next;
                        }
		    }
		}
        }
void hgraphPrint(hgraph G)  
	
        {
        vertex u,v;
	Neighbor n;
        unsigned long ix;
	Entry e;
        slistEntry se;
	printf("Printing edges of graph:\n");
	for (ix = 0; ix<G->numElements; ix++) 
		{
		e = (Entry)vectorGet(G->v,ix);
		u = (vertex)(e->val); 
        	for (se=u->adj->head; se != NULL; se=se->next)
                        {
			n = (Neighbor)(se->elem);
			v = (vertex)(n->v);
			printf("printing edge between %s and %s\n",u->key,v->key);

                        }
		}
        }

vertex hgraphFirstVertex(hgraph G) // just returns the "first" vertex in the whole graph's hash list
        {
        vertex u;
	Entry e;
	e = (Entry)vectorGet(G->v,0);
	return (vertex)(e->val); 
        }

