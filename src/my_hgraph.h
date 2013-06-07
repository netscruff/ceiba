#ifndef _MY_HGRAPH
#define _MY_HGRAPH
#include "my_slist.h"
#include "my_hash.h"
#include "my_vector.h"
#include "my_queue.h"

#define BIPARTITE 0		// define this if you want some stuff for bipartite graphs

#define INIT_BUCKETS 100 	// initial number of buckets in graph/hash

enum colorChoice {white,gray,black};
enum sideChoice {left,right};

typedef struct vertexStruct * vertex;
struct vertexStruct
	{
	char *			key;		// usually a name 
	unsigned long		compNum;	// the component number or other data
	enum colorChoice 	color;		// used for DFS and finding components
	slist			adj;		// the adjacent vertices
	void *			data;		// generic pointer to a data object
#if BIPARTITE
	enum sideChoice		side;		// which side of the bipartite graph
#endif
	};

typedef struct compStruct * Component;
struct compStruct
	{
	vertex	v;
	unsigned long size;
	};

typedef struct neighborStruct * Neighbor;
struct neighborStruct
	{
	vertex v;
	long weight;
	};

#define hgraph	Hash	// a hash graph is not allowed to have vertices with the same key

void disconnect(Vector compVec);
static Vector component2Vector(Component c);
static void compTraverse(Vector V, vertex v);
Component componentNew(vertex v, unsigned long size);
Neighbor neighborNew(vertex v, long w);
Vector components(hgraph g, int flag);
unsigned long  cutoffDFS(vertex u, unsigned long c, int cutoff); 
unsigned long  DFS(vertex u, unsigned long c);
Entry hgraphVertexKeyExists(hgraph g, char * key);
#if BIPARTITE
void   hgraphNumLRVertices(hgraph g, unsigned long *L, unsigned long *R);
vertex vertexNew(char * key, enum sideChoice side);
#else
vertex vertexNew(char * key);
#endif
void vertexFree(vertex v);
hgraph hgraphNew(void);
int hgraphAddVertex(hgraph hg, vertex v);
void hgraphAddEdgeVx(hgraph hg, vertex v1, vertex v2,long weight);
void hgraphAddEdge(hgraph hg, char *a, char *b,long w);
void hgraphDeleteNeighbor(vertex v, vertex n); 
void hgraphDeleteEdge(vertex v1, vertex v2); 
unsigned long  BFS(hgraph G, vertex s);
long hgraphEdgeWeight(vertex v1, vertex v2);
void hgraphSetEdgeWeight(vertex v1, vertex v2,long wt);
unsigned long BFSlimit(hgraph G, vertex s, unsigned long N);
unsigned long BFSmatch(hgraph G, vertex s, long matchCode);
queueStatic BFSQueue(hgraph G, vertex v);
vertex splitGraph(hgraph G, vertex s, unsigned long A_size);
void hgraphPrint(hgraph G);
vertex hgraphFirstVertex(hgraph G);
int isDisconnectVertex(hgraph G, vertex s);
int isAdjacent(vertex u, long matchCode);
#endif
