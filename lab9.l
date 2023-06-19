/*
Name: Meng-Yang Tseng 
Date: April 10 2023
lab: lab9

Purpose: lab6.l is to catch any expected symbol for lab6.y, like int, void, read. 
         They will return token or yytext depends on case.


Change(lab6): 
        * I added "yylval.string=strdup(yytext)" for T_ID token to copy the string into yacc.

Change(lab9):
        * added tokens for break and continue

*/

%{
int lineno = 1; 
int mydebug = 0;
#include "ast.h" /* added on lab6 */
#include "y.tab.h"
 
%}

%%

int                     {return (T_INT);} /* catch int keyword for token */
void                    {return (T_VOID);} /* catch void keyword for token */
read                    {return (T_READ);} /* catch read keyword for token */
if                      {return (T_IF);} /* catch if keyword for token */
else                    {return (T_ELSE);} /* catch else keyword for token */
while                   {return (T_WHILE);} /* catch while keyword for token */
write                   {return (T_WRITE);} /* catch write keyword for token */
return                  {return (T_RETURN);} /* catch return keyword for token */

break                   {return (T_BREAK);} /* catch break keyword for token */
continue                {return (T_CONTINUE);} /* catch continue keyword for token */

\".*\"                  {yylval.string=strdup(yytext); return (T_STRING);} /* catch string for token */
"<="                    {return (T_LE);} /* catch <= for token */
"<"                     {return (T_L);} /* catch < for token */
">"                     {return (T_B);} /* catch > for token */
">="                    {return (T_BE);} /* catch >= for token */
"=="                    {return (T_E);} /* catch == for token */
"!="                    {return (T_NE);} /* catch != for token */

"+"                     {return (T_PLUS);} /* catch + for token */
"-"                     {return (T_MINUS);} /* catch - for token */
"*"                     {return (T_MUTIPLE);} /* catch * for token */
"/"                     {return (T_DIVIDE);} /* catch / for token */
"%"                     {return (T_MOD);} /* added on Mar 11. catch % for token */

[a-zA-Z][a-zA-Z0-9_]*   {if (mydebug) fprintf(stderr, "ID found\n"); yylval.string=strdup(yytext); return (T_ID);} /* catch ID for token */
[0-9][0-9]*             {if (mydebug) fprintf(stderr, "Digit found\n"); 
                         yylval.value = atoi((const char*) yytext);
                         return(T_NUM);
                        } /* catch number for token */
\n                      {if (mydebug) fprintf(stderr, "cariage found %c\n",*yytext); lineno++;} /* line counter */
[,;=\%&|(){}[\]] 	    {if (mydebug) fprintf(stderr, "return a token %c\n",*yytext);return (*yytext);} /* catch expected symbol for yytext, added '-' on Mar 11 */
[ \t]                   {if (mydebug) fprintf(stderr, "White space found\n");} /* catch white space */
\/\/.*                  {if (mydebug) fprintf(stderr, "Comment found\n");} /* catch "//" */                          
%%

int yywrap(void)
{ return 1;}