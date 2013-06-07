%{

/* 
LEXER for handling NEXUS files.  M. J. Sanderson 12.14.99

Much of the complication below
is to handle possibly nested comments (using lex's nonexclusive start states).
Comments are not passed to parser at all.  This is not fully NEXUS compliant
since comments are used by some programs, but this is a bad idea!
Note that brackets are never returned as punctuation.
Attempts to match numbers first, and then alphanumeric words, but
both are returned to parser as strings.  The parser code can do conversions.


COMPILATION NOTES.

This will compile under LINUX with the following modifications

	 flex -l -i myLexer and compile the resulting C code with the -lfl library 

	The -i switch makes it case insensitive, as required by nexus.

Use yacc but add an assignment statement in its main():

	yyin=stdin; ...or the like, otherwise it doesn't seem to get
			input
	
*/

#include "nexusParser.h"
#include <string.h>
#include <stdlib.h>
int bracketcount=0;

#if 0  //  currently if this is set to 1, the flex c file will not compile...Flex doesn't let us redefine input() macro
// ... or unput(); see p 156ff in lex-yacc book
char *mystring="#nexus begin trees; tree A=((a,b,(c,d))); end; ";

#undef input
#undef unput
#define input()	(*mystring++)
#define unput(c) (*--mystring = c)
#endif


char *myinputptr;
#if 1
/* Following is nearly exactly as specified in the LEX/YACC book to take input from my own function rather than usual STDIN */
#undef YY_INPUT
#define YY_INPUT(b,r,ms) (r=my_yyinput(b,ms))
#define MIN(a,b) (((a)<(b))?(a):(b)) 
int my_yyinput(char *buf,int max_size)
{
	int n;
	n=MIN(max_size,strlen(myinputptr));
	if (n>0)
		{
		memcpy(buf,myinputptr,n);
		myinputptr+=n;
		}
	return n;
}
#endif

%}
PUNCT [-(){}\/,;:=*'"`+<>] /* Careful to put the dash first! ('-' and ']' are the only disallowed chars within char class) */
/*			   Otherwise this can match single digit integers and other weirdities */
DIGIT [0-9]
WHITE [\t\r\n\f ]
%s COMMENT NORMAL
%%
%{
	BEGIN NORMAL;
%}

"["							{BEGIN COMMENT;++bracketcount;}
<COMMENT>"[" 						{++bracketcount;}
<COMMENT>"]" 						{--bracketcount;
							if (bracketcount==0) 
								{
								BEGIN NORMAL;
								}
							}
<COMMENT>. ;
<COMMENT>{WHITE}+ ;	

<NORMAL>{WHITE}+ 	;


<NORMAL>{PUNCT}   					{return yytext[0]; }
<NORMAL>#nexus						{return NEXUS;/*printf("%s:is BEGIN\n",yytext);*/}
<NORMAL>begin  						{return NXBEGIN;/*printf("%s:is BEGIN\n",yytext);*/}
<NORMAL>end|endblock	   				{return END;/*printf("%s:is END\n",yytext);*/}
<NORMAL>tree						{return TREE;}
<NORMAL>translate					{return TRANSLATE;}
<NORMAL>mrca						{return MRCA;}
<NORMAL>\'(([^']*)(([']['])*)([^']*))*\'	 	{yylval.sval=strdup(yytext);return QUOTED;}
<NORMAL>[-+]?{DIGIT}+					{yylval.sval=strdup(yytext);return INT;}
<NORMAL>([-+]?([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?)	{yylval.sval=strdup(yytext);return REAL;}		
<NORMAL>([-+]?[0-9]*\.[0-9]+)				{yylval.sval=strdup(yytext);return REAL;}
<NORMAL>[a-zA-Z0-9_!@#$%^&~|.]+ 				{yylval.sval=strdup(yytext);return ALPHANUM;} 
<NORMAL>.	return yytext[0];

%%

