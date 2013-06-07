
#include <stdio.h>
#include "my_slist.h"
#ifndef _MYSMALLTREE
#define _MYSMALLTREE
#define SIBLOOP(c)			for (; (c); (c)=(c)->sib)
#define isTip(c)			 ( (c)->firstdesc == NULL )
#define isRoot(c)			( (c)->anc == NULL )
#define isEqual(a,b) (!(strcmp((a),(b))))  // currently this is case sensitive!

struct nodetype {
	struct nodetype		*anc;
	struct nodetype		*firstdesc;
	struct nodetype		*sib;
	char				*label;
	double				number;
	double				number2;
	unsigned long		numdesc;
	unsigned long		numdescEffective; // a different number that is used to guess sizes for layouts when clade collapsed
	unsigned long		order;
	int					nodeFlag;		// temporary register for mrca, subtreeLight, etc
	int					nodeFlagSave;	// more permanent indicator of a subtree for example
	int					isCompactNode;	// this clade will be collapsed perhaps
	void				*data;
	void				*data2;
	//void				*subtree;		// pointer to some kind of subtree
	/*GL*/float*		labelColor;
	/*GL*/float*		treeColor;
	int					branchStyle;	// 0=line; 1=branch;
	int					marked;			// for some drawing purpose; usually denotes path leading from root to a compact node
	struct nodetype		*markedAnc;		// pointer to the closest marked ancestor (or null if none)
	float				radius;			// for tube layouts, this is the radius in modeling units at this node
};

typedef         struct nodetype * node;
typedef		node * nodeArray;

unsigned long numNodesNotSub(node n); 
nodeArray newNodeArrayNotSub(node root);
int maxLabelLength(node n, char **maxLabel);  // get a pointer to longest label among leaves of this clade; ignores collapsing
unsigned long  maxorderEffective(node n);  
unsigned long  numdescEffective(node n);
unsigned long  numdescEffectiveNoSave(node n);
node lastSib(node n);
node  copyTree(node a);
void resetFlag(node n);
void resetFlagSave(node n);
node mrca(node root, slist taxonList, unsigned long *);
int isNodeDescendant(node nodeA, node nodeB);
void fatal(char *s);
node find_taxon_name(node n,char *name);
char * slurpNexus(FILE *f);
unsigned long  numNodes(node n);
unsigned long  maxorder(node n);
unsigned long  numdesc(node n);
unsigned long  numdescNoSave(node n);
void preOrderVoid(node n,void (*f)(node));
double preOrder(node n,double (*f)(node));
node nexus2rootNode(char *buffer);
node newnode(char *label, double number,double number2);
void appendSib(node L, node n);
node makeAnc(node L,char * label, double number,double number2);
void printtree(node n);
char * DupStr(char *);
double calcMaxToTip(node n);
void subtreeLight(node root, slist taxonList);
double treeLength(node n);
nodeArray newTipNodeArray(node root);
void orFlag(node n);
void andFlag(node n);
nodeArray newNodeArray(node root);

#endif
