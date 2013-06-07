%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mySmallTreeLib.h"
#include "my_vector.h"
#include "my_slist.h"
#include "my_hash.h"
#include "my_structures.h" // for DupStr

char cbuf[64];
extern node gRootNode;
extern Vector translVec; 
extern int translationTable;

slistEntry se;
slist taxList=NULL;
Hash  mrcaHash;
Entry e;
%}


%token PUNCT NEXUS NXBEGIN END TREE QUOTED
%token  INT  REAL ALPHANUM TRANSLATE MRCA 

%union {
	char *sval;
	struct nodetype *nodeval;
	struct slistType *sList;	
	long intval;
	double flval;
	}
%type <nodeval> leaf tree siblist
%type <sList> taxa_list
%type <sval> identifier taxon_name ALPHANUM number INT REAL

%%
nexus: NEXUS blocklist
	;
blocklist: block
	|blocklist block
	;
block:	NXBEGIN identifier ';' commandlist END ';'
	;
commandlist: command
	| commandlist command
	;
command: TREE identifier '=' tree ';' {gRootNode=$4;/* printf("Processed tree command\n"); printtree($4);*/}
	| TRANSLATE transl_list ';' {translationTable=1; /* set flag */} 
	| MRCA taxon_name '=' taxa_list ';' {if(!mrcaHash)mrcaHash=hashNew(50);hashInsert(mrcaHash,$2,$4,&e);taxList=NULL;}
	;


transl_list: INT taxon_name			{vectorInsertAt(translVec,(long)strtod($1,NULL), DupStr($2));}
	| transl_list ',' INT taxon_name 		{vectorInsertAt(translVec,(long)strtod($3,NULL), DupStr($4));} 
	;

taxa_list: taxon_name  {if (!taxList)taxList=slistNew();slistInsert(taxList,DupStr($1));$$=taxList;}
	| taxa_list ',' taxon_name {slistInsert(taxList,DupStr($3));$$=taxList;}
	;
 

siblist: leaf 					{$$=$1;}
	| tree 					{$$=$1;}
	| siblist ',' leaf 			{appendSib($1,$3);$$=$1;}
	| siblist ',' tree			{appendSib($1,$3);$$=$1;}
	;

/* Note that the following is context sensitive, depending on whether the TRANSLATE command has been encountered...
   Does this violate something key about the grammar rules? */

leaf: 	taxon_name		{if (!translationTable) $$=newnode($1,0.0,0.0); else $$=newnode((char*)vectorGet(translVec,strtod($1,NULL)),0.0,0.0);}
	| taxon_name ':' number		{if (!translationTable) $$=newnode($1,strtod($3,NULL),0.0);else $$=newnode((char*)vectorGet(translVec,strtod($1,NULL)),strtod($3,NULL),0.0);}
	| taxon_name ':' number ':' number		{if (!translationTable) $$=newnode($1,strtod($3,NULL),strtod($5,NULL));else $$=newnode((char*)vectorGet(translVec,strtod($1,NULL)),strtod($3,NULL),strtod($5,NULL));}
	;

tree: '(' siblist ')'  				{$$=makeAnc($2,NULL,0.0,0.0);}
	| '(' siblist ')' ':' number  		{$$=makeAnc($2,NULL,strtod($5,NULL),0.0);}
	| '(' siblist ')' taxon_name 			{$$=makeAnc($2,$4,0.0,0.0);}
	| '(' siblist ')' taxon_name ':' number 	{$$=makeAnc($2,$4,strtod($6,NULL),0.0);}
	| '(' siblist ')' taxon_name ':' number ':' number 	{$$=makeAnc($2,$4,strtod($6,NULL),strtod($8,NULL));}
        | '(' siblist ')' ':' number ':' number         {$$=makeAnc($2,NULL,strtod($5,NULL),strtod($7,NULL));}

	;
/*
The last line above was added by Andre Wehe; check to see if it works the next time you recompile...
*/

number: INT	{$$=$1;}	
	| REAL	{$$=$1;}	
	;

taxon_name: identifier		{$$=$1; /*printf("In parser: taxon_name=%s\n",$1);*/}

identifier: ALPHANUM		{$$=$1;}
	| INT			{;}
	| QUOTED		{;}
	;

%%

yyerror(char *s)
{
	fprintf(stderr,"%s\n",s);
}

