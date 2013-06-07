#include "mySmallTreeLib.h"
#include <stdio.h>
#include <stdlib.h>
// dumb main function to play with nexus tree parser. Now just reads from stdin, bypassing the buffer
// mystring below. However, see nexusLexer.l to change this

main(int argc,char * argv[])
{
node root;
char *buffer, *mystring="#nexus begin trees; tree A=(('bubba':2,bob:1,(cob:5,dob:10):1):1); end; ";
FILE *f;
if (argc !=2)
	printf("Usage: paloverde filename\n");
else
	{
	f=fopen(argv[1],"r");
	buffer = slurpNexus(f);
	root=nexus2rootNode(buffer);
	printtree(root);
	printf("%s\n",buffer);
	}
}
