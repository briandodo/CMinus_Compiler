
/* 
* Name: Meng Yang Tseng
* Date: April 10 2023
* lab: lab9
* purpose: header file including user define data type for AST

* changed(lab6): 
      * added any ASTtype which I created in lab6.y's grammar rules
      * added any operators which I created in lab6.y's grammar rules
      * modified from booltype to inttype (AST_MY_DATA_TYPE)
* changed(lab7):
      * none
* changed(lab9);
      * added label into astnodetype
      * added A_BREAK into ASTtype
*/

#include<stdio.h>
#include<malloc.h>
#include "symtable.h" //added on lab 7

#ifndef AST_H
#define AST_H
int mydebug;

/* define the enumerated types for the AST.  THis is used to tell us what 
sort of production rule we came across */

enum ASTtype {
   A_FUNCTIONDEC,
   A_VARDEC,
   A_VAR, 
   A_COMPOUND,
   A_WRITE,
   A_READ,
   A_NUM,
   A_EXPR,
   A_PARAMDEC,
   A_SELESTMT,
   A_SELEBODY,
   A_EXPRSTMT,
   A_CALL,
   A_ARGLIST,
   A_WHILE,
   A_ASSISTMT,
   A_RETRUNSTMT,
	   //missing
      // added any node which I created in lab6.y 
   A_BREAK // added on lab 9
};

// Math Operators

enum AST_OPERATORS {
   A_PLUS,
   A_MINUS,
   A_LESSEQUAL,
   A_LESS,
   A_BIG,
   A_BIGEQUAL,
   A_EQUAL,
   A_NOTEQUAL,
   A_MUTIPLE,
   A_DIVIDE,
   A_UMINUS,
   A_MOD
	   //missing
      // added any operators which I created in lab6.y's rules
      // removed A_TIMES on lab 9
      // added A_MOD on lab 9(it was in wrong position)
};

enum AST_MY_DATA_TYPE {
   A_INTTYPE, /* modified from booltype to inttype*/
   A_VOIDTYPE

};

/* define a type AST node which will hold pointers to AST structs that will
   allow us to represent the parsed code 
*/

typedef struct ASTnodetype
{
     enum ASTtype type;
     enum AST_OPERATORS operator;
     char * name;
     char * label; // for label, added on lab 9
     int linecount; // for emit.c to print line
     int value;
     enum AST_MY_DATA_TYPE my_data_type;
     struct SymbTab *symbol;
     ///.. missing, fixed it on lab 7
     struct ASTnodetype *s1,*s2, *next ; /* used for holding IF and WHILE components -- not very descriptive */
} ASTnode;


/* uses malloc to create an ASTnode and passes back the heap address of the newley created node */
ASTnode *ASTCreateNode(enum ASTtype mytype);

void PT(int howmany);


/*  Print out the abstract syntax tree */
void ASTprint(int level,ASTnode *p);
/* Params checking */
int check_params(ASTnode *actuals, ASTnode *formals);

#endif // of AST_H
