#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mySmallTreeLib.h"
#include "my_vector.h"
#include "my_slist.h"
#include "my_hash.h"
#include "my_structures.h"
#include <math.h>


static void newTipNodeArrayHelper(nodeArray A, node n, long *ix);
static void newNodeArrayNotSubHelper(nodeArray A, node n);


Vector translVec;
int translationTable=0;
node gRootNode;
node gn1;
long gix;
static void newNodeArrayHelper(nodeArray A, node n);



nodeArray newNodeArrayNotSub(node root) // returns all nodes except descendants of compact nodes
{
long N;
nodeArray A;
N=numNodesNotSub(root);
A=(nodeArray)malloc(N*sizeof(node));
gix=0;
newNodeArrayNotSubHelper(A,root);
return A;
}
static void newNodeArrayNotSubHelper(nodeArray A, node n)
{
node child;
A[gix++]=n;
if (n->isCompactNode) return;
child = n->firstdesc;
SIBLOOP(child)
	newNodeArrayNotSubHelper(A,child);
}




nodeArray newNodeArray(node root)
{
long N;
nodeArray A;
N=numNodes(root);
A=(nodeArray)malloc(N*sizeof(node));
gix=0;
newNodeArrayHelper(A,root);
return A;
}
static void newNodeArrayHelper(nodeArray A, node n)
{
node child;
A[gix++]=n;
child = n->firstdesc;
SIBLOOP(child)
	newNodeArrayHelper(A,child);
}


nodeArray newTipNodeArray(node root)
// OK as long as this function gets setup only once; else static init in helper will screw up on subseq calls
{
long N,ix;
nodeArray A;
N=numdesc(root);
A=(nodeArray)malloc(N*sizeof(node));
ix=0;
newTipNodeArrayHelper(A,root,&ix);
return A;
}
static void newTipNodeArrayHelper(nodeArray A, node n, long *ix)
{
node child;
if (isTip(n))
	{
	A[*ix]=n;
	++*ix;
	}
//printf("ix=%li\n",*ix);
	
child = n->firstdesc;
SIBLOOP(child)
	newTipNodeArrayHelper(A,child,ix);
}

node nexus2rootNode(char *buffer)

// As of now, we configure to read from a file throughout...[but see the following note if you want to change]

// Depending on how nexusLexer.l is configured, this routine either invokes a parser on STDIN or invokes a parser
// based on the string pointed to by buffer. STDIN or string must contain a valid nexus file with a tree block and one
// tree. It is parsed and a pointer to the root node of a tree structure is returned.

{
extern char* myinputptr;
translVec=vectorNew(100);
myinputptr=buffer;
yyparse();
return gRootNode;
}

#define	INIT_BUFFER_SIZE	10000 

/* reads from a file stream a nexus file and returns the string buffer */

char * slurpNexus (FILE * inFileStream)

{
	char	*buffer;
	int		c;
	long	count=0,i=0,bufSize=INIT_BUFFER_SIZE;

	
	buffer=(char*)malloc(bufSize*sizeof(char));

	while ((c=fgetc(inFileStream)) != EOF)	/* have to define c as int so that EOF can be detected*/
		{
		if (i >= bufSize-1) /* have to save room for terminating null */
			{
			bufSize*=1.6;
			buffer=(char*)realloc(buffer,bufSize*sizeof(char));
			if (!buffer)
				printf("Failure to grow buffer in slurpNexus\n");
			}
		buffer[i]=c;
		++i;
		
		}
		buffer[i]='\0';
return buffer;	
}

unsigned long numNodes(node n)
{

	unsigned long sum=0;
	node child;
	child=n->firstdesc;
	SIBLOOP(child) 
		sum += numNodes(child); /* add one for each child and all that children's*/
	return (1+sum);	/* the 1 is to count this node, which must be internal */
}

unsigned long numNodesNotSub(node n) // num nodes except for descendants of compact nodes
{

	unsigned long sum=0;
	node child;
	child=n->firstdesc;
	if (n->isCompactNode) return (1);
	SIBLOOP(child) 
		sum += numNodesNotSub(child); /* add one for each child and all that children's*/
	return (1+sum);	/* the 1 is to count this node, which must be internal */
}


/***********************************************************************************/
void preOrderVoid(node n,void (*f)(node))
{
	node child;
	(*f)(n);
	if (!isTip(n))
		{
		child=n->firstdesc;
		SIBLOOP(child) 
			preOrderVoid(child,f);
		}
	return ;	
}
double preOrder(node n,double (*f)(node))
{
	double sum=0;
	node child;
	sum+=(*f)(n);
	if (!isTip(n))
		{
		child=n->firstdesc;
		SIBLOOP(child) 
			sum += preOrder(child,f);
		}
	return (sum);	
}
int maxLabelLength(node n, char **maxLabel)  // get a pointer to longest label among leaves of this clade; 
	{
	node child;
	char *bestMaxLabel;
	int max=0,len;
	if (isTip(n))
		{
		*maxLabel = n->label;
printf("Leaf = %s length = %i\n",n->label,strlen(n->label));
		return strlen(n->label);
		}

	else if (!isRoot(n) && n->isCompactNode)
		{
		if (n->label ) // a root node might be compact but it is never a leaf in a rendered tree
			{
			*maxLabel = n->label;
			return strlen(n->label);
			}
		else
			{
			*maxLabel = "";
			return 0;
			}
		}

	else
		{
		child=n->firstdesc;
		SIBLOOP(child) 
			{
			len=maxLabelLength(child, maxLabel);
			if (len > max)
				{
				max = len;
				bestMaxLabel=*maxLabel;
				}
				
			}
		*maxLabel=bestMaxLabel;
		return max;
		}
	
	}


/***********************************************************************************/
unsigned long  maxorderEffective(node n)  
// the maximum number of nodes between this node and a descendant tip
// corrected in case of internal named node
{
        unsigned long  max,temp;
        node child;
        if (!n) return(-1);
        if (isTip(n)  ) 
			{n->order=0; return (0);}
		else
			{
			if (n->isCompactNode)
				{
				n->order=0;
				return 0;
				}
			max=0;
			child=n->firstdesc;
			SIBLOOP(child) {
							temp=maxorderEffective(child);
							if (temp > max) max = temp;
							}
			n->order=max+1;
			return (max+1);
			}
}
unsigned long  maxorder(node n)  // the number of descendant nodes along the longest path between this node and its leaves,
								// not including this node. 
{
        unsigned long  max,temp;
        node child;
        if (!n) return(-1);
        if (isTip(n)  ) {n->order=0; return (0);}
        max=0;
        child=n->firstdesc;
        SIBLOOP(child) {
                        temp=maxorder(child);
                        if (temp > max) max = temp;
                        }
        n->order=max+1;
        return (max+1);
}

unsigned long  numdescNoSave(node n)

/* determines the number of leaves descended from every node and stores them at node */

{
	unsigned long sum=0;
	node child;
	if (!n) return(-1);
	if (isTip(n)) 
		{
		return (1);
		}
	child=n->firstdesc;
	SIBLOOP(child) 
		sum+=numdescNoSave(child);
	return (sum);
}




#if 0
unsigned long  numdescEffective(node n)

// Determine number of descendant leaves, counting any compact node as a leaf and neglecting it's descendeants

// This version only traverses from root down to compact nodes and returns (doesn't pass through
// compact nodes and into subtrees). It stores numdescEffective=1 at those compact nodes.

{
	unsigned long sum=0;
	node child;
	if (!n) return(-1);
	if (isTip(n) || n->isCompactNode) 
		{
		n->numdescEffective=1; 
		return (1);
		}
	child=n->firstdesc;
	SIBLOOP(child) 
		sum+=numdescEffective(child);
	return (n->numdescEffective=sum);

}
#else

unsigned long  numdescEffective(node n)

// Determine number of descendant leaves, counting any compact node as a leaf and neglecting it's descendeants

// This version does traverses from root down through compact nodes and out to leaves.
// It stores the actual value of numdescEffective at those compact nodes--rather than 1.

// OK, I think this fucking finally handles leaves, internal nodes, etc. correctly.
// NEED TO DO SAME FOR NUMDESC

#if 0
{
	unsigned long sum=0;
	node child;
	if (!n) return(-1);
	if (isTip(n)) 
		{
//		n->numdescEffective=1; 
		n->numdescEffective=0; 
		return (1);
		}
	child=n->firstdesc;
	SIBLOOP(child) 
		sum+=numdescEffective(child);
	n->numdescEffective=sum;
	if (n->isCompactNode)
		return 1;
	else
		return sum;

}
#endif
{
	unsigned long sum=0;
	node child;
	if (!n) return(-1);
	child=n->firstdesc;
	SIBLOOP(child) 
		{
		if (isTip(child))
			++sum;
		else if (child->isCompactNode)
			{
			++sum;
			(void)numdescEffective(child);
			}
		else
			sum+=numdescEffective(child);
		}
	n->numdescEffective=sum;
	return sum;

}




#endif

unsigned long  numdesc(node n)

/* determines the number of leaves descended from every node and stores them at node */

{
	unsigned long sum=0;
	node child;
	if (!n) return(-1);
	if (isTip(n)) 
		{
		n->numdesc=1; 
		return (1);
		}
	child=n->firstdesc;
	SIBLOOP(child) 
		sum+=numdesc(child);
	n->numdesc=sum;
	return (sum);
}

node newnode(char *label, double number,double number2)

// Pass label=NULL if don't want to assign a label to this node; otherwise pass the string

	{
	node n;
	n=(node)malloc(sizeof(struct nodetype));
	if (n)
		{
		n->label=NULL; // bug fix!
		n->anc=NULL;
		n->firstdesc=NULL;
		n->sib=NULL;
		if (label != NULL)
			n->label=DupStr(label);
		n->number=number;
		n->number2=number2;
		n->nodeFlag=0;
		n->nodeFlagSave=0;
		n->data=NULL;
		n->data2=NULL;
		n->isCompactNode=0;
		n->branchStyle=0; // style = lines by default
		n->marked=0;
		n->markedAnc=NULL;
		n->radius=0.0;
		return n;
		}
	else
		return NULL;
	}

/* find the last sib of given node */

node lastSib(node n)
	{
	if (n)
		while(n->sib)
			n=n->sib;
	return n;
	}




/* add a node as the sib of a list of sibs */

void appendSib(node L, node n)
	{
	if (L)
		{
		while(L->sib)
			L=L->sib;
		L->sib=n;
		}
	return;
	}

/* make a new node that is ancestral to a list of sibs, L */

node makeAnc(node L, char * label, double number,double number2)
	{
	node n;
	n=newnode(label,number,number2);
	if (n)
		{
		n->firstdesc=L;
		L->anc=n;
		while (L->sib) /* set all the sibs ancestor */
			{
			L=L->sib;
			L->anc=n;
			}
		return n;
		}
	else
		return NULL;
	}

void printtree(node n)
	{
	node child;
	printf("Node %s: number=%f\n",n->label,n->number);
	if (child=n->firstdesc)
	   for (;child;child=child->sib)
		printtree(child);
	return;
	}

node find_taxon_name(node n,char *name)
/* returns the node that has a taxon name that matches 'name' or NULL
if it fails to find */


{
        node child, found_node;

        if (n->label)
                if (isEqual(name,n->label))
                        return n;
        child=n->firstdesc;
        SIBLOOP(child)
                if (found_node=find_taxon_name(child,name) )
                        return found_node;
        return NULL;
}
void subtreeLight(node root, slist taxonList)

// For the subtree induced by the taxonList (extending down to its MRCA), set the nodeFlag for all its nodes
// Note this calls mrca, which resets all nodeflags to 0 before proceeding...
{
node MRCA,p,p1;
unsigned long nMatched;
slistEntry e;
char * tax;
if (taxonList->numElements < 2)
	fatal("Must have two or more taxa in subtreeLight command\n");
if (!(MRCA = mrca(root,taxonList,&nMatched)))
	fatal("MRCA not found in subtreeLight()\n");
e=taxonList->head;
while (e)
	{
	tax = e->elem;
//printf("%s\n",tax);
	p1 = find_taxon_name(root,tax);
	for (p=p1;p!=MRCA;p=p->anc)
		p->nodeFlag=1;
	MRCA->nodeFlag=1;
	e=e->next;
	}
}
node mrca(node root, slist taxonList, unsigned long *nMatched)

// NOTE. This routine resets each node's nodeFlag to 0 upon exit.
// Also note that in older versions, I destructively changed the input taxonList; now it just traverses it.
// Something to beware of elsewhere!
{

char * tax1, *tax2;
node 	p1, p2, p, theMRCA;
slistEntry e,e1,e2;
*nMatched = 0;
if (taxonList->numElements < 2)
	{
	printf("Must have two or more taxa in mrca command\n");
	return;
	}

for (e1=taxonList->head; e1; e1 = e1->next)
	{
	p1 = find_taxon_name(root,e1->elem);
	if (p1) break;
	}
if (!p1) 
	{
	printf("Warning: No names in the mrca command were found in the tree\n");
	return NULL; // got to here with p1 null means none of the names matched...
	}
*nMatched = 1;
theMRCA=p1;
resetFlag(root); // initialize the tree to all 0's

for (p=p1;p;p=p->anc)
        p->nodeFlag=1;

//while (taxonList->numElements)
e2=e1; // sets e2 to the last one found above
while (e2=e2->next)
	{
	tax2=e2->elem;
	p2 = find_taxon_name(root,tax2);
	if (!p2)
		{
		//printf("Taxon name %s not found on tree in mrca command\n",tax2);
		continue;
		}
	else
		{
		++(*nMatched);
		for (p=p2;p;p=p->anc)
				if (p->nodeFlag==1)
						if (isNodeDescendant(theMRCA,p)) 
					{
					theMRCA=p; // if the current MRCA is actually younger than p, make p the MRCA. 
					break;	// go on to the next taxon in list
					}
			}
	}

resetFlag(root); 
return theMRCA;
}

/**********************************************************************/

int isNodeDescendant(node nodeA, node nodeB)
/*
 * Returns 1 if nodeA is the descendant of nodeB or is nodeB; 0 if not (note that A and B might not be either) 
 */

{
node n;
for(n=nodeA;n;n=n->anc)
        /* worst case, terminates when node = NULL at ancestor of root */
        if (n==nodeB) return 1;
return 0;
}

void andFlag(node n)
// for each node in the tree, takes the nodeFlag value, logical 'ors' it with the nodeFlagSave value and stores it in the latter
{
	node child;

	n->nodeFlagSave = n->nodeFlag && n->nodeFlagSave;
	child=n->firstdesc;
	SIBLOOP(child) 
		andFlag(child); 
}
void orFlag(node n)
// for each node in the tree, takes the nodeFlag value, logical 'ors' it with the nodeFlagSave value and stores it in the latter
{
	node child;

	n->nodeFlagSave = n->nodeFlag || n->nodeFlagSave;
	child=n->firstdesc;
	SIBLOOP(child) 
		orFlag(child); 
}

void resetFlagSave(node n)
{
	node child;
	n->nodeFlagSave=0;
	child=n->firstdesc;
	SIBLOOP(child) 
		resetFlagSave(child); 
}
void resetFlag(node n)
{
	node child;
	n->nodeFlag=0;
	child=n->firstdesc;
	SIBLOOP(child) 
		resetFlag(child); 
}

void fatal(char * msg)
{
printf("%s\n",msg);
exit(1);
}


node  copyTree(node a)
// returns a node that is either a tip, or the root of a properly formatted tree, but its ancestor and sibs are undefined
{
node child,first,newfirst,newn,n,prev;
//printf("Numdesc:%i\n",numdesc(a));
newn = newnode(a->label,a->number,a->number2);

newn -> isCompactNode = a -> isCompactNode; // added 10.5.2012

//printf("New Label created:%s\n",newn->label);
//copy....
if(!isTip(a))
	{
	first=a->firstdesc;
	newfirst=copyTree(first);
//printf("Inserted as first descendant Label:%s\n",newfirst->label);
	newn->firstdesc=newfirst;
	newfirst->anc = newn;
	prev=newfirst;
	child=first->sib; // start loop with the second sib in the sib list...
	SIBLOOP(child)
		{
		n = copyTree(child);
//printf("Inserted as sib Label:%s\n",n->label);
		prev->sib=n;
		prev=prev->sib;
		n->anc = newn;
		}
	}
//printf("NumdescNew:%i\n",numdesc(newn));
return  newn;
}

double calcMaxToTip(node n)

/* Calculates maximum path length distance from root to tip when lengths are available.
*/

{
        double max=0.0,temp,thisLength;
        node  child;
        if (!n) return(0.0);

        if (isRoot(n))
                thisLength=0.0;
        else
				thisLength=n->number;        /* don't add length under the root */
        if (isTip(n))
                return (thisLength);  /* treat a tip and a compact node the same way */
        child=n->firstdesc;
        SIBLOOP(child) {
                        temp=calcMaxToTip(child);
                        if (temp > max) max = temp;
                        }
        return thisLength+max;
}
double treeLength(node n)

/* Sums the branch lengths over tree assuming they are stored in the 'number' field.
   Does not include the length of the branch subtending the root */

{
	node child;
	double dur;
	if (isRoot(n))
		dur=0.0;
	else
		dur=n->number;
	child=n->firstdesc;
	SIBLOOP(child)
		dur+=treeLength(child);
	return dur;
}

